#define read_list(p);    create_list(p); restore_element(4); title_op(SHOW);

#define move_up()       line_or_move_up(3)

#define move_down()     move_or_line_down((int)(SCREEN_LEN - 2))

#define get_replace()   (void) readprompt(replace_item,M_REPLACE, SEARCHLEN)

#define copy_to_end(Result, line, from) \
        strsub(Result, line, from, (strlen(line) - from + 1))
 
#define entered(p)      \
    ((p)->foldline == START_ENTER_FOLD || (p)->foldline == START_ENTER_FILED)
 
#define filed_or_fold(p)        \
                 ((p)->foldline == START_FOLD || (p)->foldline == START_FILED)
 
#define total_indent(p)         ((p)->UU.U1.indent + (p)->UU.U1.fold_indent)
 
#define line_of(Result, parts)  strcpy((Result), (parts)->strng)
 
#define copy_parts_to_line(parts, line)   strcpy((line), (parts)->strng)
 
#define check_fold(p)     (Start_of_fold = p, \
                           End_of_fold = Start_of_fold->other_end, \
                           Line_after_fold = Start_of_fold->next, \
                           End_of_fold->next = Line_after_fold)
