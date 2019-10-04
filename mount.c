
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>

// Goel Biju - 2019


// Declarations - prototype functions
int mount_readonly(char* mountPoint);
int mount_readwrite(char* mountPoint);
int mount_bind(char* sourceDirectory, char* targetDirectory);

void unmount(char* targetDirectory);


int main(int argc, char* argv[])
{
    printf("Mount\n");

    // Perform the set of actions required:
    // 1. Mount specified location as writable
    // 2. Create specified folder at mounted location
    // 3. Re-mount specified location as read-only
    // 4. Create specified folder in a default writable location (i.e. /tmp)
    // 5. Bind mount the mounted location and the temporary location.

    // sourceDirectory can be treated as the folder in /tmp
    // targetDirectory can be treated as the folder created in the mount location

    // mount_readonly();
    // mount_readwrite();
    // mount_bind("", "");
    // unmount("withfiles");


    // Get the real uid/gid from the user who called it by checking the environmental variables.
    const char* sudoUid = getenv("SUDO_UID");
    const char* sudoGid = getenv("SUDO_GID");

    if (sudoUid == NULL || sudoGid == NULL)
    {
        printf("* Could not get SUDO_UID/SUDO_GID as getenv for SUDO_UID/SUDO_GID returned NULL.\nRun program as sudo?\n");
        return -1;
    }

    uid_t callingUid;
    gid_t callingGid;

    callingUid = atoi(sudoUid);
    callingGid = atoi(sudoGid);

    //printf("True UID (of calling user): %u\n", callingUid);
    //printf("True GID (of calling group): %u\n", callingGid);


    // Handle the arguments that are passed to the program.
    int i;
    if (argc == 3)
    {
        printf("Number of arguments supplied: %d\n", argc - 1);

        // TODO: Needs specific arguments e.g. -p (mount point), -f (folder name), -l (mount location).
        // Get the arguments needed and their values; remove mount bind with umount (-u argument?).
        //for (i = 1; i < argc; i++)
        //{
        //    char* argument = argv[i];
        //    printf("Argument: %s\n", argument);
        //}

        char* mountPoint = argv[1];
        char* mountLocation = argv[2];

        printf("- Mount point specified: %s\n", mountPoint);
        printf("- Mount location specified to create folder and mount bind: %s\n", mountLocation);


        // Mount specified location as writable.
        int res;

        res = mount_readwrite(mountPoint);
        if (res < 0)
        {
             printf("* Failed to set %s as read/write, result: %d\nRun program as sudo?\n", mountPoint, res);
             return -1;
        }

        printf("Successfully set %s as read/write.\n", mountPoint);


        // Use 0755 to allow for readable by other than root.
        res = mkdir(mountLocation, 0755);
        if (res <  0)
        {
            printf("* Failed to create directory at following path: %s\n", mountLocation);
            mount_readonly(mountPoint);

            return -1;
        }

        printf("Created mount directory %s (permission: 0755), result: %d\n", mountLocation, res);


        // Create a temporary directory securely.
        char templateDir[] = "/tmp/mntdir.XXXXXX";
        char* tmpDirName = mkdtemp(templateDir);

        if (tmpDirName == NULL)
        {
            printf("* Failed to create temporary directory with mkdtemp.\n");
            mount_readonly(mountPoint);

            return -1;
        }

        printf("Created tmp directory name: %s\n", tmpDirName);
        // TODO: Delete this temporary directory after use.

        // Use the real uid and gid to change the ownership of the directory.
        if (chown(tmpDirName, callingUid, callingGid) == -1)
        {
            printf("* Failed to change the ownership of the temporary directory (/tmp/%s) to %d (uid): %d (gid).\n", tmpDirName, callingUid, callingGid);
            return -1;
        }

        printf("Successfully changed ownership of the temporary directory (/tmp/%s) to %d (uid): %d (gid).\n", tmpDirName, callingUid, callingGid);


        // Re-mount location as read-only.
        res = mount_readonly(mountPoint);
        if (res < 0)
        {
             printf("* Failed to set %s as read-only.\n", mountPoint);
             return -1;
        }

        printf("Successfully set %s as read-only.\n", mountPoint);


        // Bind mount the temporary location with the mount location.
        res = mount_bind(tmpDirName, mountLocation);
        if (res < 0)
        {
            printf("* Failed to bind mount %s to %s, result: %d\n", tmpDirName, mountLocation, res);
            return -1;
        }

        printf("Successfully bind mounted %s with %s.\n", tmpDirName, mountLocation);
    }
    else if (argc < 3)
    {
        printf("* Please provide the mount point and the mount location (including the directory to create), in that order.\nE.g. ./mount /opt /opt/software/cmake\n");
        return -1;
    }
    else
    {
        printf("* No arguments passed to the program.\n");
        return -1;
    }


    printf("- Operation complete.\n");
    return 0;
}


int mount_readonly(char* mountPoint)
{
    char* fileType = "nfs4";

    printf("- Re-mounting %s as read-only (MS_REMOUNT and MS_RDONLY)\n", mountPoint);
    int result = mount("", mountPoint, fileType, MS_REMOUNT | MS_RDONLY, "");
    printf("- Re-mounting result: %d\n", result);

    return result;
}


int mount_readwrite(char* mountPoint)
{
    char* fileType = "nfs4";

    printf("- Re-mounting %s as read and writable (MS_REMOUNT) with file type: %s\n", mountPoint, fileType);
    int result = mount("", mountPoint, fileType, MS_REMOUNT, "");
    printf("- Re-mounting result: %d\n", result);

    return result;
}


int mount_bind(char* sourceDirectory, char* targetDirectory)
{
    char* fileType = "nfs4";

    printf("- Bind mounting %s to %s (MS_BIND) with file type: %s\n", sourceDirectory, targetDirectory, fileType);
    int result = mount(sourceDirectory, targetDirectory, fileType, MS_BIND, "");
    printf("- Bind Mounting result: %d\n", result);

    return result;
}


void unmount(char* targetDirectory)
{
    int result = umount(targetDirectory);
    printf("Unmounting result: %d\n", result);
}
