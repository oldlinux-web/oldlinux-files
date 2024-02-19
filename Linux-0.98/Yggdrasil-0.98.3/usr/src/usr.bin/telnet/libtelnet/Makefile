OBJS=auth.o enc_des.o encrypt.o genget.o getent.o kerberos.o \
	kerberos5.o krb_des.o misc.o
TARGET= libtelnet.a

$(TARGET): $(OBJS)
	ar cq $(TARGET) $(OBJS)
	ranlib $(TARGET)

clean:
	rm $(TARGET) $(OBJS) *~
