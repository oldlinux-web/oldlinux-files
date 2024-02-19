/*---------------------------------------------------------------------------

  unimplod.c

  The Imploding algorithm is actually a combination of two distinct algor-
  ithms.  The first algorithm compresses repeated byte sequences using a
  sliding dictionary.  The second algorithm is used to compress the encoding
  of the sliding dictionary ouput, using multiple Shannon-Fano trees.

  ---------------------------------------------------------------------------

  Copyrights:  see accompanying file "COPYING" in UnZip source distribution.

  ---------------------------------------------------------------------------*/


#include "unzip.h"


/***********************/
/*  UnImplode Defines */
/***********************/

#define LITVALS     256
#define DISTVALS    64
#define LENVALS     64
#define MAXSF       LITVALS



/************************/
/*  UnImplode Typedefs */
/************************/

typedef struct sf_entry {
    byte Value;
    byte BitLength;
} sf_entry;

typedef struct sf_tree {        /* a shannon-fano "tree" (table) */
    sf_entry entry[MAXSF];
    int entries;
    int MaxLength;
} sf_tree;

typedef struct sf_node {        /* node in a true shannon-fano tree */
    UWORD left;                 /* 0 means leaf node */
    UWORD right;                /*   or value if leaf node */
} sf_node;



/********************************/
/*  UnImplode Global Variables */
/********************************/

/* s-f storage is shared with that used by other comp. methods */

sf_tree lit_tree;
sf_tree length_tree;
sf_tree distance_tree;
sf_node *lit_nodes = (sf_node *) prefix_of;     /* 2*LITVALS nodes */
#ifdef MACOS
sf_node *length_nodes ;  /* 2*LENVALS nodes */
sf_node *distance_nodes ;    /* 2*DISTVALS nodes */
#else
sf_node *length_nodes = (sf_node *) suffix_of;  /* 2*LENVALS nodes */
sf_node *distance_nodes = (sf_node *) stack;    /* 2*DISTVALS nodes */
#endif
boolean lit_tree_present;
boolean eightK_dictionary;
int minimum_match_length;
int dict_bits;



/*****************************************/
/*  UnImplode Local Function Prototypes */
/*****************************************/

static void LoadTrees __((void));
static void LoadTree __((sf_tree * tree, int treesize, sf_node * nodes));
static void ReadLengths __((sf_tree * tree));
static void SortLengths __((sf_tree * tree));
static void GenerateTrees __((sf_tree * tree, sf_node * nodes));
static void ReadTree __((register sf_node * nodes, int *dest));





/**************************/
/*  Function unImplode() */
/**************************/

void unImplode()
 /* expand imploded data */
{
    register int srcix;
    register int Length;
    register int limit;
    int lout;
    int Distance;

    LoadTrees();

#ifdef DEBUG
    printf("\n");
#endif
    while ((!zipeof) && ((outpos + outcnt) < ucsize)) {
        READBIT(1, lout);

        if (lout != 0) {        /* encoded data is literal data */
            if (lit_tree_present) {     /* use Literal Shannon-Fano tree */
                ReadTree(lit_nodes, &lout);
#ifdef DEBUG
                printf("lit=%d\n", lout);
#endif
            } else
                READBIT(8, lout);

            OUTB(lout);
        } else {                /* encoded data is sliding dictionary match */
            READBIT(dict_bits, Distance);

            ReadTree(distance_nodes, &lout);
#ifdef DEBUG
            printf("d=%5d (%2d,%3d)", (lout << dict_bits) | Distance, lout,
                   Distance);
#endif
            Distance |= (lout << dict_bits);
            /* using the Distance Shannon-Fano tree, read and decode the
               upper 6 bits of the Distance value */

            ReadTree(length_nodes, &lout);
            Length = lout;
#ifdef DEBUG
            printf("\tl=%3d\n", Length);
#endif
            /* using the Length Shannon-Fano tree, read and decode the
               Length value */

            if (Length == 63) {
                READBIT(8, lout);
                Length += lout;
            }
            Length += minimum_match_length;

            /* move backwards Distance+1 bytes in the output stream, and copy
              Length characters from this position to the output stream.
              (if this position is before the start of the output stream,
              then assume that all the data before the start of the output
              stream is filled with zeros.  Requires initializing outbuf
              for each file.) */

            srcix = (outcnt - (Distance + 1)) & (OUTBUFSIZ - 1);
            limit = OUTBUFSIZ - Length;
            if ((srcix <= limit) && (outcnt < limit)) {
                outcnt += Length;
                while (Length--)
                    *outptr++ = outbuf[srcix++];
            } else {
                while (Length--) {
                    OUTB(outbuf[srcix++]);
                    srcix &= OUTBUFSIZ - 1;
                }
            }
        }
    }
}





/**************************/
/*  Function LoadTrees() */
/**************************/

static void LoadTrees()
{
    eightK_dictionary = (lrec.general_purpose_bit_flag & 0x02) != 0;    /* bit 1 */
    lit_tree_present = (lrec.general_purpose_bit_flag & 0x04) != 0;     /* bit 2 */

    if (eightK_dictionary)
        dict_bits = 7;
    else
        dict_bits = 6;

    if (lit_tree_present) {
        minimum_match_length = 3;
        LoadTree(&lit_tree, 256, lit_nodes);
    } else
        minimum_match_length = 2;

    LoadTree(&length_tree, 64, length_nodes);
    LoadTree(&distance_tree, 64, distance_nodes);
}





/*************************/
/*  Function LoadTree() */
/*************************/

static void LoadTree(tree, treesize, nodes)
sf_tree *tree;
int treesize;
sf_node *nodes;
 /* allocate and load a shannon-fano tree from the compressed file */
{
    tree->entries = treesize;
    ReadLengths(tree);
    SortLengths(tree);
    GenerateTrees(tree, nodes);
}





/****************************/
/*  Function ReadLengths() */
/****************************/

static void ReadLengths(tree)
sf_tree *tree;
{
    int treeBytes;
    int i;
    int num, len;

    /* get number of bytes in compressed tree */
    READBIT(8, treeBytes);
    treeBytes++;
    i = 0;

    tree->MaxLength = 0;

/* High 4 bits: Number of values at this bit length + 1. (1 - 16)
 * Low  4 bits: Bit Length needed to represent value + 1. (1 - 16)
 */
    while (treeBytes > 0) {
        READBIT(4, len);
        len++;
        READBIT(4, num);
        num++;

        while (num > 0) {
            if (len > tree->MaxLength)
                tree->MaxLength = len;
            tree->entry[i].BitLength = len;
            tree->entry[i].Value = i;
            i++;
            num--;
        }

        treeBytes--;
    }
}





/****************************/
/*  Function SortLengths() */
/****************************/

static void SortLengths(tree)
sf_tree *tree;
 /* Sort the Bit Lengths in ascending order, while retaining the order
   of the original lengths stored in the file */
{
    register sf_entry *ejm1;    /* entry[j - 1] */
    register int j;
    register sf_entry *entry;
    register int i;
    sf_entry tmp;
    int entries;
    unsigned a, b;

    entry = &tree->entry[0];
    entries = tree->entries;

    for (i = 0; ++i < entries;) {
        tmp = entry[i];
        b = tmp.BitLength;
        j = i;
        while ((j > 0)
               && ((a = (ejm1 = &entry[j - 1])->BitLength) >= b)) {
            if ((a == b) && (ejm1->Value <= tmp.Value))
                break;
            *(ejm1 + 1) = *ejm1;/* entry[j] = entry[j - 1] */
            --j;
        }
        entry[j] = tmp;
    }
}





/******************************/
/*  Function GenerateTrees() */
/******************************/

static void GenerateTrees(tree, nodes)
sf_tree *tree;
sf_node *nodes;
 /* Generate the Shannon-Fano trees */
{
    int codelen, i, j, lvlstart, next, parents;

    i = tree->entries - 1;      /* either 255 or 63 */
    lvlstart = next = 1;

    /* believe it or not, there may be a 1-bit code */

    for (codelen = tree->MaxLength; codelen >= 1; --codelen) {

        /* create leaf nodes at level <codelen> */

        while ((i >= 0) && (tree->entry[i].BitLength == codelen)) {
            nodes[next].left = 0;
            nodes[next].right = tree->entry[i].Value;
            ++next;
            --i;
        }

        /* create parent nodes for all nodes at level <codelen>,
           but don't create the root node here */

        parents = next;
        if (codelen > 1) {
            for (j = lvlstart; j <= parents - 2; j += 2) {
                nodes[next].left = j;
                nodes[next].right = j + 1;
                ++next;
            }
        }
        lvlstart = parents;
    }

    /* create root node */

    nodes[0].left = next - 2;
    nodes[0].right = next - 1;
}





/************************/
/*  Function ReadTree() */
/************************/

#ifndef ASM

static void ReadTree(nodes, dest)
register sf_node *nodes;
int *dest;
 /* read next byte using a shannon-fano tree */
{
    register int cur;
    register int left;
    UWORD b;

    for (cur = 0;;) {
        if ((left = nodes[cur].left) == 0) {
            *dest = nodes[cur].right;
            return;
        }
        READBIT(1, b);
        cur = (b ? nodes[cur].right : left);
    }
}

#endif                          /* !ASM */
