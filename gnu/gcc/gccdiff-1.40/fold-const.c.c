*** originals/fold-const.c	Sat Aug 17 21:29:11 1991
--- fold-const.c	Sat Aug 17 21:29:53 1991
***************
*** 138,143 ****
--- 138,158 ----
  	}
      }
  }
+ #if (defined(CROSS_MINIX) || defined(NATIVE_MINIX))
+ #ifdef	m68k	/* Only needed for MINIX-68k - ajm */
+ tree
+ cast_to_sizeof_type(t)
+ tree t;
+ {
+    extern int flag_traditional;
+ 
+    t = copy_node(t);
+    TREE_TYPE(t) = flag_traditional ? integer_type_node : unsigned_type_node;
+    force_fit_type(t);
+    return t;
+ }
+ #endif	/* m68k */
+ #endif 	/* CROSS_MINIX || NATIVE_MINIX */
  
  /* Add two 64-bit integers with 64-bit result.
     Each argument is given as two `int' pieces.
