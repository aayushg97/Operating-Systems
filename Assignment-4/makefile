all:	test1	test2	test3	test4

test1:	myfs.h	test_1.c	
		gcc test_1.c -o test1 -lpthread -w
test2:	myfs.h	test_2.c	
		gcc test_2.c -o test2 -lpthread -w
test3:	myfs.h	test_3.c	
		gcc test_3.c -o test3 -lpthread -w
test4:	myfs.h	test_4.c	
		gcc test_4.c -o test4 -lpthread -w

clean:	test1	test2	test3	test4
		rm test1
		rm test2
		rm test3
		rm test4