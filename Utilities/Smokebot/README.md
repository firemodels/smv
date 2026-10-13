# Smokebot: A Continuous Integration Tool for Smokeview

Smokebot is a verification test script that can be run at regular intervals as part of a continuous integration program. At NIST, the script is run by a pseudo-user named `smokebot` on a linux cluster each night and whenever updates are made the fds or smv repos. The pseudo-user `smokebot` updates the various repositories in the GitHub project named `firemodels`, compiles FDS and Smokeview, runs the verification cases, generates smokeview images, and builds all of the smokeview manuals. The entire process takes an hour to complete.

## Set-Up

The following steps only need to be done once. The exact phrasing of the commands are for the NIST linux cluster named spark. You might need to modify the path and module names.

1. Clone the repositories that are included in the GitHub organization called `firemodels`: `bot`, `cfast`, `fds`, `fig` and `smv`. Clone `bot` first, then cd into `bot/Scripts` and type `./setup_repos.sh -a` . This will clone all the other repos needed in the same directory as `bot` (or you can clone each repo in the same way as you cloned `bot`).

2. Ensure that the following software packages are installed on the system:

    * Intel Fortran and C compilers
    * Gnu Fortran compiler
    * LaTeX (TeX Live distribution), be sure to make this the default LaTeX in the system-wide PATH

3. smokebot uses email notifications for build status updates. Ensure that outbound emails can be sent using the `mail` command.

4. Install libraries for Smokeview. On CentOS, you can use the following command:
   ```
   yum install mesa-libGL-devel mesa-libGLU-devel libXmu-devel libXi-devel xorg-x11-server-Xvfb
   ```

5. Add the following lines (or the equivalent if you are using a different Intel compiler) to your `~/.bashrc` file:
    ```
    source /opt/intel/oneapi/setvars.sh >& /dev/null
    # - needed to build smokeview
    export IFORT_COMPILER_LIB=/opt/intel/oneapi/compiler/latest/linux/compiler/lib/intel64_lin
    ulimit -s unlimited
    ```
6. Setup passwordless SSH for the your account. Generate SSH keys and ensure that the head node can SSH into all of the compute nodes. Also, make sure that your account information is propagated across all compute nodes.

7. Ensure that a queue named `smokebot` is created and enabled. Test the `qstat` command.  If you use some other queue say batch then use `-q batch` when running smokebot.

8. By default, smokebot sends email to the email address configured for your bot repo (output of command `git config user.email` ) .  If you wish email to go to different email addresses, create a file named $HOME/.smokebot/firebot_email_list.sh for some `user1` and `user2` (or more) that looks like:

```
#!/bin/bash

# General mailing list for smokebot status report
mailToFDS="user1@host1.com, user2@host2.com"
```

## Running Smokebot

The script `smokebot.sh` is run using the wrapper script `run_smokebot.sh`. This script uses a lock file that ensures multiple instances of smokebot do not run at the same time. To see the various options associated with running smokebot, type
```
./run_smokebot.sh -H
```
Important things to consider: do you want to test your own local changes, or update your repositories from the central repository? Do you want to use Intel MPI or Open MPI? Do you want to skip certain stages of the process?

You can start smokebot at some future time using `crontab` with an entry like the following:
```
PATH=/bin:/usr/bin:/usr/local/bin:$PATH
MAILTO=""
# .---------------- minute (0 - 59)
# |   .------------- hour (0 - 23)
# |   |   .---------- day of month (1 - 31)
# |   |   |   .------- month (1 - 12) OR jan,feb,mar,apr ...
# |   |   |   |  .----- day of week (0 - 7) (Sunday=0 or 7)  OR sun,mon,tue,wed,thu,fri,sat
# |   |   |   |  |
# *   *   *   *  *  command to be executed
# generate movies at 4:01AM
1 4 * * *    cd $HOME/Models_central/bot/Smokebot ; bash -lc  "./run_smokebot.sh -J -u -c -U -q smokebot -M -W http://spark.el.nist.gov/smokebot -w /var/www/html/smokebot > /dev/null"

# run smokebot if FDS or Smokeview source has changed in last 5 minutes
*/5 * * * * cd $HOME/FireModels_central/bot/Smokebot ; bash -lc "./run_smokebot.sh -J -u -c -U -q smokebot -a -W http://spark.el.nist.gov/smokebot -w /var/www/html/smokebot > /dev/null"
```

The output from smokebot is written into the directory called `output` which is in the same directory as the `smokebot.sh` script itself. When smokebot completes, email should be sent to the specified list of addresses.

## Updating Comparison Images

Smokebot compares images it generates with a corresponding set of base images located in the fig repo.
To update the base images on a Linux or Mac computer:

Assume bot, fig, smv repos etc are under $HOME/FireModels_fork

1.  bring fig repo up to date
```
    cd $HOME/FireModels_fork/fig
    git remote update
    git merge firemodels/master
    git merge origin/master
    git push origin master
```
2. compare images
```
  cd $HOME/FireModels_fork/bot/Smokebot
  ../scripts/update_repo_images.sh -r path_to_smokebot_root  (at NIST: ~smokebot/Firemodels_clone )
```
  where `path_to_smokebot_root` is the root directory containing the bot/Smokebot directory that generated the images.  

3. add updated images to fig repo
  ```cd $HOME/FireModels_fork/bot/fig```
  follow the usual procedure to incorporate updated fig repo images into your repo and github 
  ie stage, commit, push and do a pull request



At NIST we would type the following command to generate the new base set of images

```
cd $HOME/FireModels_fork/bot/Smokebot
../scripts/update_repo_images -r ~smokebot/Firebot_clone
```
