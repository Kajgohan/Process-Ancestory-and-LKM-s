Ken Desrosiers and Jack Hogan -- kdesrosiers & jfhogan

In order to run this program successfully, enter the directory, and run the command make. You need to then insert the two modules part1.ko and part2.ko (first one, test it, remove it, then the other). Then, enter the sub-directory called testcases, and run make. This will create three tests: part1, part2input, and part2fork.

part1: This test case shows that part 1 works correctly. It creates a file and called open on it, calls open on an already existing file, closes it, and reads two files: one containing the word 'VIRUS', and one that does not have it. These results can be seen in part1log.txt

part2input: This test case allows the user in input a pid to test out. Process 1 should not have any parent, but after doing research, we found out that the OS gives it a parent ID  of 0 just as a placeholder. With this in mind, all processes are said to have ancestors leading up to 1 and 0. We have one small bug where it says that process 1 and process 2 (kthreadd) are ancestors and siblings of each other. For every other process, it works just fine.

part2fork: Finally, in this user test, we create a child process and run an ancestry search on its parent. The results are accurate, because it then says that the process has one child: the child process. This shows that is works.

Attached are also the syslogs to prove that all of the parts work correctly.

Thank you!

Ken and Jack
