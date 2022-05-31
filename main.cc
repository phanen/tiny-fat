
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

// void test2()
// {
//     test1();

// }

// test create delete ls cd
void test1()
{
    fs_create("fa", TYPE_FILE);
    fs_create("fb", TYPE_FILE);
    fs_create("fc", TYPE_FILE);
    fs_create("fd", TYPE_FILE);
    fs_create("fe", TYPE_FILE);
    fs_ls();

    fs_delete("fa");
    fs_delete("fb");
    fs_delete("fc");
    fs_ls();

    fs_create("da", TYPE_DIR);
    fs_create("db", TYPE_DIR);
    fs_create("dc", TYPE_DIR);
    fs_ls();

    int sz = 65;
    char *buf = new char[sz]; //
    for (int i = 0; i < sz; i++)
        buf[i] = i;

    int fd1 = fs_open("fd");
    cout << "fd1: " << fd1 << endl;
    int len = fs_write(fd1, buf, sz);
    cout << "write len: " << len << endl;

    len = fs_read(fd1, buf, sz);
    cout << "read len: " << len << endl;
    for (int i = 0; i < len; i++)
        cout << int(buf[i]) << " ";
    cout << endl;
    fs_ls();

    fs_cd("da");
    fs_ls();

    fs_create("dda", TYPE_DIR);
    fs_create("dfa", TYPE_FILE);
    fs_create("dfb", TYPE_FILE);

    fs_create("dfc", TYPE_FILE);
    fs_create("dfd", TYPE_FILE);
    fs_create("dfe", TYPE_FILE);
    fs_create("dff", TYPE_FILE);

    fs_create("ddg", TYPE_FILE);
    fs_create("ddh", TYPE_FILE);
    fs_create("ddi", TYPE_FILE);
    fs_create("ddj", TYPE_DIR);

    fs_ls();

    fs_cd("dda");
    fs_create("ddda", TYPE_DIR);
    fs_create("dddb", TYPE_DIR);
    fs_ls();

    fs_cd("ddda");
    fs_create("dddda", TYPE_DIR);
    fs_create("ddddb", TYPE_DIR);
    // fs_create("dddfc", TYPE_DIR);
    fs_create("dddfc", TYPE_FILE);
    fs_ls();

    sz = 96;
    for (int i = 0; i <= 0xff; i++)
        buf[i] = i;
    int fd2 = fs_open("dddfc");
    cout << "fd2: " << fd2 << endl;

    len = fs_write(fd2, buf, sz);
    cout << "write len: " << len << endl;
    len = fs_read(fd2, buf, sz);
    cout << "read len: " << len << endl;
    for (int i = 0; i < len; i++)
        cout << int(buf[i]) << " ";
    cout << endl;
    fs_close(fd2);

    fs_ls();
    fs_cd("..");
    fs_ls();
    fs_close(fd1);
    fd1 = fs_open("fd");
    sz = 25;
    len = fs_read(fd1, buf, sz);
    cout << "read len: " << len << endl;
    for (int i = 0; i < len; i++)
        cout << int(buf[i]) << " ";
    cout << endl;

    fs_cd("da");
    fs_ls();
    fs_cd("dda");
    fs_ls();
    fs_create("dddc", TYPE_FILE);
    fs_create("dddd", TYPE_FILE);
    fs_create("ddde", TYPE_FILE);
    fs_create("dddf", TYPE_FILE);
    fs_ls();

    fs_cd("ddda");
    fs_ls();

    fd2 = fs_open("dddfc");
    sz = 66;
    len = fs_read(fd2, buf, sz);
    cout << "read len: " << len << endl;
    for (int i = 0; i < len; i++)
        cout << int(buf[i]) << " ";
    cout << endl;
    fs_close(fd2);

    fs_delete("dddfc");
    fs_ls();
    fs_create("dddfc", TYPE_FILE);
    fs_create("dddfd", TYPE_FILE);
    fs_ls();

    fd2 = fs_open("dddfd");
    fs_ls();
    sz = 66;
    len = fs_read(fd2, buf, sz);
    cout << "read len: " << len << endl;
    for (int i = 0; i < len; i++)
        cout << int(buf[i]) << " ";
    cout << endl;
    cout << "test 1 pass" << endl;
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
