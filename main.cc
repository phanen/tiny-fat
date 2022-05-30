
#include "kernel.h"

#include <iostream>
using namespace std;
void disk_formatter();

void test0();
void test1();
void test2();
void test3();

int main()
{
    disk_formatter();
    fs_boot();

    // test0();
    test1();

    fs_shutdown();
}
void test2()
{
}

// test create delete ls
void test1()
{
    fs_create("fa", TYPE_FILE);
    fs_create("fb", TYPE_FILE);
    fs_create("fc", TYPE_FILE);
    fs_create("fd", TYPE_FILE);
    fs_create("fe", TYPE_FILE);
    cout << 1 << endl;
    fs_ls();

    fs_delete("fa");
    fs_delete("fb");
    fs_ls();
    fs_delete("fc");
    fs_ls();

    fs_create("da", TYPE_DIR);
    fs_create("db", TYPE_DIR);
    fs_ls();
    fs_create("dc", TYPE_DIR);
}

// test read write
void test0()
{
    // fs_create("asd", TYPE_DIR);

    fs_create("a", TYPE_FILE);
    fs_create("b", TYPE_FILE);

    int fd1 = fs_open("a");
    int fd2 = fs_open("b");
    cout << fd1 << endl;
    cout << fd2 << endl;

    int len;

    u8_t *buf = new u8_t[0x100];
    for (int i = 0; i <= 0xff; i++)
        buf[i] = i;

    int sz = 256;
    len = fs_write(fd1, buf, sz);
    cout << "len: " << len << endl;
    for (int i = 0; i <= 0xff; i++)
        buf[i] = 0xff - i;

    len = fs_write(fd2, buf, sz);
    cout << "len: " << len << endl;
    for (int i = 0; i <= 0xff; i++)
        buf[i] = 0xff - i;

    len = fs_read(fd1, buf, sz);
    cout << "len: " << len << endl;
    for (int i = 0; i < sz; i++)
        cout << int(buf[i]) << ' ';
    // cout << buf[i] << ' ';
    cout << endl;
    cout << "aa " << endl;

    len = fs_read(fd2, buf, sz);
    cout << "len: " << len << endl;
    for (int i = 0; i < sz; i++)
        cout << int(buf[i]) << ' ';
    // cout << buf[i] << ' ';
    cout << endl;
    cout << "aa " << endl;
    delete[] buf;
    fs_close(fd1);
    fs_close(fd2);
}
