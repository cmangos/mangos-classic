# The DB_Tools subdirectory
This directory is a place to store sql-scripts for database work and maintenance.

The core team will provide sql-files that will not change any content in any database, but will output data that is considered incorrect or obsolete.

We will use these subdirectories:
* WorldDB -- To output content we consider bad filled in the world-database (default: mangos)
* CharDB -- To output content we consider bad filled in the characters-database (default: characters)
* RealmDB -- To output content we consider bad filled in the realmd-database (default: realmd)

You can add additional Folders as you wish to store own sql-files for your own purposes.
These folders are also processable with the provided scripts.

You can use the sql-files just like any normal sql-files, however for easy use and batch-mode there are shipped two helper scripts:

1. UseDB_Tools.sh
2. UseDB_Tools_Interactive.sh

The only difference between these two scripts is, that the second will always run in "interactive" mode.

These scripts will help you executing the sql-files of the subdirectories.

## Introduction to the UseDB_Tools[_Interactive].sh helper scripts
The scripts will execute sql-scripts of the subfolders and log the output in files named
`<subfolder>.log`.

You can run them with the following parameters:
* -h display help
This will give a short information about the available options.
* -i for interactive mode
In the interactive mode you will be asked about which subfolders and which files to process.
* -f DIR to force processing only the subfolder DIR
This option allows you to use the script only on one of the given subfolders. The subfolder must be provided as plain name without additional path information.
* -q run in quiet mode
When this option is set, you will not be bothered with too much information.
* -d DB to force using the database DB instead of the one named in the configuration

## Configuration files in the subfolders
When the script is run it will try to guess the used database for a subfolder based on its name.
With a normal setup, you will not have to worry about this at all!

However in case that you use non default user or databasenames, or wish to use the tools on special databases, you can modify the files named

`db_info.conf`

located in each of a processed subfolder to your needs.

These config files are pretty clear, they only store information about database, database-user and the user's password.

This information is used by the UseDB_Tools[_Interactive].sh helper scripts for the subfolder, the content of these files is only stored on your side and not safed with git or similar.

Note however that storing sensible information is a security risk, so you should do so only on trusted systems (or when the information is non-critical)

## Suggested format of the sql-files
As the sql-files are processed automatically (with the mysql command), they must be proper sql syntax.

For automated use it is also suggested to follow these guidelines:
* First line should be a comment giving rough information about what the sql-commands in this file will do.
Note that the first line will always be displayed in the console as information about what is supposed to happen in the script.
* Additional comments that are emphasized with `-- --` will be output in non-quiet mode as additional information to the user.
* Further comments will be ignored
* The actual SQL statements should be placed at the end of the file.

An example file might look like:

    -- This is an example. It will ensure that Hogger is spawned

    -- -- This comment is displayed without the -q (quiet) option.
    -- -- This SQL file ensures that the most important NPC is spawned in the world.
    -- -- Is Hogger spawned?

    -- This is a comment for the sql-file only, the script will not touch it.
    -- The following query ensures that Hogger is spawned!
    -- Do not forget the semicolon at the end
    SELECT guid, id, position_x, position_y, position_z, map FROM creature WHERE id=448;
