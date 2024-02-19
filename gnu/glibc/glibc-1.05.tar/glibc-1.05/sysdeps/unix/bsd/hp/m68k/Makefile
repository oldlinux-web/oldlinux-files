$(objpfx)%.o: %.S
	$(CC) $(CPPFLAGS) -E $< \
	| sed 's/(@@@Hash-Here@@@)/#/g' | $(AS) $(ASFLAGS) -o $@
.S-rule := t
