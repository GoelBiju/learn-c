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

#define MAX_PATH 4096
#define MOUNT_POINT "/opt"
#define MOUNT_POINT_LENGTH 4


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

    // Get the real uid/gid from the user who called the program.
    uid_t callingUid = getuid();
    gid_t callingGid = getgid();

    printf("- True UID (of calling user): %u\n", callingUid);
    printf("- True GID (of calling group): %u\n", callingGid);

    // Handle the arguments that are passed to the program.
    if (argc == 2)
    {
        int res;
        printf("\n- Number of arguments supplied: %d\n", argc - 1);

        char* mountLocation = argv[1];
        printf("- Mount location specified to create folder and mount bind: %s\n", mountLocation);

        // Perform some checks to ensure the mount location argument provided is valid:
        //  1. Get the real path from the mounted location.
        //  2. (NOT NEEDED?) Check the length of the mount location provided and ensure it is of the correct size.
        //  3. Check that the start of mount location provided is matching to the start of the mount point.

        char targetLocation[MAX_PATH];
        
        // NOTE: Running realpath will check if the path provided exists or not.
        //       We want to canonical path for creating, so we only check that the 
        //       target location is still not empty.
        realpath(mountLocation, targetLocation);
        if (strlen(targetLocation) == 0)
        {
            errsv = errno;
            printf("* Failed to get the real-path from mount location %s.\n", mountLocation);
            printf("Error Code: %d (%s)\n", errsv, strerror(errsv));

            return errsv;
        }

        printf("- Got real-path of provided mount location: %s\n", targetLocation);


        // Check to see if the length of the mount point is correct.
        // size_t mountPointLength = strlen(MOUNT_POINT);
        // size_t mountLocationLength = strlen(mountLocation);
        
        // Ensure the length of the location provided is greater than 
        // the minimum length required (/opt/).
        // if (mountLocationLength <= mountPointLength + 1) {
        //     printf("* Please specify a mount location under %s.\n", MOUNT_POINT);
        //     return 2;
        // }

        // printf("- Length of mount point: %lu\n", mountPointLength);
        // printf("- Length of mount location: %lu\n", mountLocationLength);
        // printf("- Mount location specified to create folder and mount bind: %s\n", mountLocation);


        // Ensure the mount location has /opt as it's prefix by concatenate
        // the mount-point with a forward-slash into the prefix array.
        char mountLocationPrefix[MOUNT_POINT_LENGTH + 1] = MOUNT_POINT;
        strcat(mountLocationPrefix, "/");
        printf("- Location provided will be checked for mount location prefix: %s\n", mountLocationPrefix);

        // Compare to see if the given location has the mount point prefix.
        if (strncmp(targetLocation, mountLocationPrefix, MOUNT_POINT_LENGTH + 1) != 0) 
        {
            printf("* Please specify a mount location under %s.\n", MOUNT_POINT);

            return 2;
        }

        printf("- Specified mount location is under %s.\n", MOUNT_POINT);


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

                // Mount our mount point (/opt) as writable since the directory does not exist.
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

                // Ensure the uid and gid of the created directory is set as root.
                if (chown(targetLocation, 0, 0) == -1)
                {
                    errsv = errno;
                    printf("* Failed to change the ownership of the temporary directory (%s) to 0 (uid): 0 (gid).\n", targetLocation);
                    printf("Error Code: %d (%s)\n", errsv, strerror(errsv));
                    
                    return errsv;
                }

                printf("Successfully changed ownership of created mount location (%s) to 0 (uid): 0 (gid).\n", targetLocation);
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
    else if (argc < 2 || argc > 2)  
    {
        printf("* Please provide a mount location, with the mounted directory %s as prefix, for bind mounting.\n", MOUNT_POINT);
        return 2;
    }
    else 
    {
        printf("* No valid arguments passed to the program.\n");
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
