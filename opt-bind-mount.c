/**
 * Opt-Bind-Mount
 * 
 * Goel Biju - 2019
 * 
 * Compiling:
 *  1. sudo gcc opt-bind-mount.c -o opt-bind-mount
 *  2. sudo chmod 4755 opt-bind-mount
 *  3. ./opt-bind-mount provide-directory-name
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MOUNT_POINT "/opt"


// Declarations - prototype functions
int mount_readonly(char* mountPoint);
int mount_readwrite(char* mountPoint);
int mount_bind(char* sourceDirectory, char* targetDirectory);

// void unmount(char* targetDirectory);


int main(int argc, char* argv[]) 
{
    int errsv;

    printf("Opt-Bind-Mount\n");
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
    

    // Get the real uid/gid from the user who called the program.
    uid_t callingUid = getuid();
    gid_t callingGid = getgid();

    printf("- True UID (of calling user): %u\n", callingUid);
    printf("- True GID (of calling group): %u\n", callingGid);

    // Set the uid to root in order to perform the actions required by the program.
    if (setuid(0) < 0) {
        errsv = errno;
        printf("* Setting the UID to root (0) did not work, are you sure the executable has the setuid bit set?\n");
        printf("Error Code: %d (%s)\n", errsv, strerror(errsv));

        return errsv;
    }

    printf("- Set UID to root (0): %u\n", getuid());
    

    // Handle the arguments that are passed to the program.
    // int i;
    if (argc == 2)
    {
        int res;

        printf("\nNumber of arguments supplied: %d\n", argc - 1);

        char* mountLocation = argv[1];
        printf("- Mount location specified to create folder and mount bind: %s/%s\n", MOUNT_POINT, mountLocation);


        // Mount our mount point (/opt) as writable if the specified mount location does not exist.
        char targetLocation[100];
        strcpy(targetLocation, MOUNT_POINT);
        strncat(targetLocation, "/", 1);
        strncat(targetLocation, mountLocation, strlen(mountLocation));


        // Check to see if the specified location already exists, if so, use that.
        // If it does not exist then create it.
        struct stat s;
        res = stat(targetLocation, &s);
        if (res < 0) 
        {
            errsv = errno;
            if (errsv != ENOENT) 
            {
                printf("* Caught an error whilst checking if the mount location already exists.\n");
                printf("Error Code: %d (%s)\n", errsv, strerror(errsv));
            }
            else 
            {
                printf("- Mount location does not exist under %s.\n", MOUNT_POINT);

                // Since the directory does not exist, we need set the mount point to read/write.
                res = mount_readwrite(MOUNT_POINT);
                if (res < 0)
                {
                    errsv = errno;
                    printf("* Failed to set %s as read/write, result: %d\n", MOUNT_POINT, res);
                    printf("Error Code: %d (%s)\n", errsv, strerror(errsv));

                    return errsv;
                }

                printf("Successfully set %s as read/write.\n", MOUNT_POINT);

                // We can now create the directory.
                // Use 0755 to allow for readable by other than root.
                res = mkdir(targetLocation, 0755);
                if (res <  0)
                {
                    errsv = errno;
                    printf("* Failed to create directory at following path: %s\n", targetLocation);
                    printf("Error Code: %d (%s)\n", errsv, strerror(errsv));

                    printf("- Ensuring %s is mounted read-only.\n", MOUNT_POINT);
                    mount_readonly(MOUNT_POINT);

                    return errsv;
                }

                printf("Created mount directory %s (permission: 0755), result: %d\n", targetLocation, res);
            }
        }
        else 
        {
            if (!S_ISDIR(s.st_mode)) 
            {
                printf("* The mount location specified exists, but it is NOT a directory.\n");
                return 2;
            }

            printf("Mount location (%s/%s) already exists and is a directory. This location will be used.\n", MOUNT_POINT, mountLocation);
        }


        // Create a temporary directory securely.
        char templateDir[] = "/tmp/mntdir.XXXXXX";
        char* tmpDirName = mkdtemp(templateDir);

        if (tmpDirName == NULL)
        {
            errsv = errno;
            printf("* Failed to create temporary directory with mkdtemp.\n");
            printf("Error Code: %d (%s)\n", errsv, strerror(errsv));
            
            mount_readonly(MOUNT_POINT);
        
            return errsv;
        }
		
        printf("Created temporary directory: %s\n", tmpDirName);
	    // NOTE: Delete this temporary directory after use. 


	    // Use the real uid and gid to change the ownership of the directory.
        if (chown(tmpDirName, callingUid, callingGid) == -1)
	    {
            errsv = errno;
            printf("* Failed to change the ownership of the temporary directory (%s) to %d (uid): %d (gid).\n", tmpDirName, callingUid, callingGid);
            printf("Error Code: %d (%s)\n", errsv, strerror(errsv));
            
            return errsv;
	    }

        printf("Successfully changed ownership of the temporary directory (%s) to %d (uid): %d (gid).\n", tmpDirName, callingUid, callingGid);


	    // Re-mount location as read-only.
        res = mount_readonly(MOUNT_POINT);
	    if (res < 0)
	    {
            errsv = errno;
            printf("* Failed to set %s as read-only.\n", MOUNT_POINT);
            printf("Error Code: %d (%s)\n", errsv, strerror(errsv));
            
            return errsv;	     
	    }

	    printf("Successfully set %s as read-only.\n", MOUNT_POINT);


	    // Bind mount the temporary location with the mount location.
        res = mount_bind(tmpDirName, targetLocation);
	    if (res < 0)
	    {
            errsv = errno;
            printf("* Failed to bind mount %s to %s, result: %d\n", tmpDirName, targetLocation, res);
            printf("Error Code: %d (%s)\n", errsv, strerror(errsv));
	        
            return errsv;
	    }

        printf("Successfully bind mounted %s with %s.\n", tmpDirName, targetLocation);
    }
    else if (argc < 2) 
    {
        printf("* Please provide the mount location, within the mounted directory %s, for bind mounting.\n", MOUNT_POINT);
        return 2;
    }
    else 
    {
        printf("* No arguments passed to the program.\n");
    	return 2;
    }

    printf("- Operation complete.\n");
    return 0;
}

 
int mount_readonly(char* mountPoint)
{
    printf("- Re-mounting %s as read-only (MS_REMOUNT and MS_RDONLY)\n", mountPoint);
    int result = mount(NULL, mountPoint, NULL, MS_REMOUNT | MS_RDONLY, NULL);

    return result;
}


int mount_readwrite(char* mountPoint)
{
    printf("- Re-mounting %s as read/write (MS_REMOUNT)\n", mountPoint);  
    int result = mount(NULL, mountPoint, NULL, MS_REMOUNT, NULL);

    return result;
}


int mount_bind(char* sourceDirectory, char* targetDirectory) 
{
    printf("- Bind mounting %s to %s (MS_BIND)\n", sourceDirectory, targetDirectory);
    int result = mount(sourceDirectory, targetDirectory, NULL, MS_BIND, NULL);

    return result;
}


// void unmount(char* targetDirectory)
// {
//     int result = umount(targetDirectory);
//     // printf("Unmounting result: %d\n", result);
// }
