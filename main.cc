
#include "kernel.h"

#include <iostream>
int main()
{
    using namespace std;
    void disk_formatter();

    disk_formatter();

    fs_boot();
    // fs_create("asd", TYPE_DIR);
    fs_create("as", TYPE_FILE);
    fs_create("asd", TYPE_FILE);

    int fd1 = fs_open("as");
    int fd2 = fs_open("asd");

    cout << fd1 << endl;
    cout << fd2 << endl;

    fs_delete("as");
    fs_delete("asd");
    fs_delete("asd");

    using namespace std;
    // fs_create();
    fs_shutdown();
    // fs_ls();
    // fs_mkdir("a");
    // fs_cd();
    // fs_create("a", 1);
}