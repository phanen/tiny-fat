
#include "kernel.h"

int main()
{
    void disk_formatter();

    disk_formatter();

    fs_boot();
    fs_create("asd", TYPE_DIR);
    // fs_create();

    // fs_create();
    fs_shutdown();
    // fs_ls();
    // fs_mkdir("a");
    // fs_cd();
    // fs_create("a", 1);
}