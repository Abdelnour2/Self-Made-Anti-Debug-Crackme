# Self-Made Crackme with Anti-Debug Defense - Documentation
## Date: Monday 23 February 2026

After the last 5 Crackmes, I wanted a step up! The first thing I thought of is making a Crackme that detects if it's being debugged and closes on that! I made the program in C (code on my github)

After that, I started cracking it as I don't have the code. I ran the program and I saw that it takes a password and prints "Invalid input" if I entered string characters as the password, and "ACCESS DENIED" if I entered numbers instead! So the password must be a number!

I went to x64dbg, and ran the program, it immediately fired the "DEBUGGER DETECTED! Exiting for security..." string and exited after 2 seconds. So I started looking for ways to bypass it.. I used the search for strings functionality to search for the "Invalid input" string to locate the main code block and I found it immediately!

After that I started reading the ASM instructions line by line. Upon reading, I noticed these instructions:
```asm
mov rax,qword ptr ds:[<IsDebuggerPresent>]
call rax
je main.7FF73B4C1773
mov rcx,rax
call <JMP.&puts>
mov ecx,7D0
mov rax,qword ptr ds:[<Sleep>]
call rax
mov eax,1
jmp main.7FF73B4C180E
```

First, the program is loading the IsDebuggerPresent function to RAX, then calls it. If the debugger is present, the "test eax, eax" will fail and will skip the je instruction.. Here the program will print the "DEBUGGER DETECTED" string and call the Sleep function after giving it 7D0 (2000) amount of ms, then close the program.

je main.7FF73B4C1773, will jump to the block where it asks for the password. and jmp main.7FF73B4C180E will jump to the end of the program where it closes.

So here, I started thinking of how I can trigger the je instruction even if the debugger is found.. at first I thought about making it a jmp instruction so that whatever the result of the test instruction it will go to the main block. I researched the hex value of the JMP instruction and I found that it is E9. I changed the value of je (74) to jmp (E9), but it didn't go as I intended, since the address of the jump also changed even though I didn't touch it.. so I ran the program anyways and yeah it didn't work.. Here I started researching to see why it didn't work and I found that je (74) is a short jump (8-bit) and E9 is a near jump (32-bit) so when I made that edit, the CPU tried to read the new extra bytes which basically corrupted the code. And I found that the short jump equivalent to the jmp instruction is EB instead of E9.

I returned to the debugger and changed je (74) to jmp (eb) and it worked! I bypassed the anti-debugging technique. Now it is time to see what the password is!

Reading the main code ASM instructions, I noticed that after getting the user input, there is a test to see if the input is a number or not:
```asm
lea rax,qword ptr ss:[rbp-4]
lea rcx,qword ptr ds:[7FF73B4D40A9]
mov rdx,rax
call main.7FF73B4C2F70 ---> calling the check function
cmp eax,1 ---> checking if the checking function returned 1 which means yes a number
je main.7FF73B4C17BF ---> continue execution
lea rax,qword ptr ds:[7FF73B4D40AC] --> loading the "Invalid input" string
```

After the test is successful, the program takes the input and give it to a function:
```asm
mov eax,dword ptr ss:[rbp-4]
mov ecx,eax
call main.7FF73B4C1710
```

I stepped into the function to see what's happening, And this is the code of the function:
```asm
push rbp
mov rbp,rsp
sub rsp,10
mov dword ptr ss:[rbp+10],ecx
mov eax,dword ptr ss:[rbp+10]
xor eax,1337
add eax,64
mov dword ptr ss:[rbp-4],eax
mov eax,dword ptr ss:[rbp-4]
add rsp,10
pop rbp
ret
```

4) The program is taking the user input from ecx and putting it in rbp+10
5) Then taking the input from rbp+10 and putting it in eax
6) Then a xor calculation is being performed eax xor 0x1337 (4919 in decimal), and the result will be stored in eax
7) After the xor, 0x64 (100 in decimal) is being added to eax

So the user input is being xored by 4919, then 100 will be added to the result

returning to the main code after the function:
```asm
cmp eax, 1770
```

The program is comparing the result of the function in eax, to 0x1770 (6000 in decimal). If it is not equal, the program jumps to where "ACCESS DENIED" is printed then exits the program, but if it is equal to 6000. "ACCESS GRANTED" will be printed and the program will exit.

So at the end, I conclude that the user needs to enter x where (x ^ 4919) + 100 = 6000 or (x ^ 0x1337) + 0x64 = 0x1770

So x can be 1083!

I didn't stop here. After researching a bit, I learned that there is another way to bypass the anti-debug technique.. I learned that IsDebuggerPresent() doesn't actually search the computer for a debugger. It simply checks a single byte in a data structure called the PEB (Process Environment Block). If that byte is 0x01, the program knows it's being watched!

PEB is a header in memory that contains info about the process: loaded modules, command-line arguments, and many more, one of these information is the BeingDebugged flag. In x64 Windows, the pointer to the PEB is stored in a special segment register called GS. Specifically located at GS:[0X60].

To locate that address I ran the command: dump gs:[0x60] in x64dbg. This command loads the PEB into the dump. The BeingDebugged flag is the 3rd byte (PEB+0x02). All I need to do is change the value of it from 0x01 (being debugged) to 0x00 (not being debugged) and that's it!

But in the professional world, this is not enough! After more researching, I've learned that professional software knows that this is an easy bypass. So they use deeper checks inside PEB. Other things professionals do are:
- Checking the NtGlobalFlag found at PEB+0xBC: When a debugger is attached, Windows sets specific bits here (0x70) that indicate "heap tail checking" and "free fill". Professionals zero this out too.
- ProcessHeap Flags found at PEB+x30: The PEB points to the Default Process Heap. In a debugger, the Heap's internal flags (ForceFlags) change. Professionals edit these flags to make the heap look natural.