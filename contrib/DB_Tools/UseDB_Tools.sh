#!/bin/bash
# This tool is used to work upon sql files stored in subfolders of this folder
#
#
# Known options
# -i for interactive mode
# -h display help
# -f DIR Only process directory
# -q quiet mode
#  d DB Work upon database DB
#
function show_help
{
    echo "You can run this tool with the following options:"
    echo "  -i        To enable interactive mode"
    echo "  -q        To be quiet about much of the output"
    echo "  -f DIR    To only work on the folder DIR"
    echo "  -h or -?  To display this help"
    echo "  -d DB     Force use database DB"
}

SCRIPT_PATH="${BASH_SOURCE[0]}"
SCRIPT_DIR=`dirname "$SCRIPT_PATH"`

MYSQL="mysql"
DATABASE=
DB_USER=
DB_PASS=

TMP_FILE="tmp_UseDBTools_$RANDOM"

#options
interactive=0
quiet=0
only_directory=""
force_database=""

while getopts "h?iqf:d:" opt; do
    case "$opt" in
    h|\?)
        show_help
        exit 0
        ;;
    q)  quiet=1
        ;;
    f)  only_directory=$OPTARG
        ;;
    d)  force_database=$OPTARG
        ;;
    i)  interactive=1
        ;;
    esac
done

# $1 Must be a directory
function create_db_config
{
    if [ "$interactive" = "1" ]
    then
        echo "Which DB should be operated within directory $1?"
        read line
        DATABASE=$line
        echo "Which user should be used for database $DATABASE?"
        read line
        DB_USER=$line
        echo "Which password should be used for database $DATABASE and user $DB_USER?"
        read line
        DB_PASS=$line
    else
        #Guess based on starting string
        DB_USER="mangos"
        DB_PASS="mangos"
        case `basename "$1"` in
            world* | World*)
                DATABASE="mangos"
                ;;
            char* | Char*)
                DATABASE="characters"
                ;;
            realm* | Realm*)
                DATABASE="realmd"
                ;;
            script* | Script*)
                DATABASE="scriptdev2"
                ;;
            *)
                echo "Unable to guess a proper name for the database, use 'mangos' as default."
                DATABASE="mangos"
                ;;
        esac
    fi

    cat << EOF > "$1"/db_info.conf
DATABASE="$DATABASE"
DB_USER="$DB_USER"
DB_PASS="$DB_PASS"

EOF
}

# $1 Must be a subdirectory
function work_directory
{
    log_file="${1}.log"

    echo "UseDB_Tools: Start working on directory ${1}" | tee "$log_file"
    echo "" | tee -a "$log_file"

    #Create if needed and get DB-config
    if [ ! -e "$1"/db_info.conf ]
    then
        create_db_config "$1"
    fi

    . "$1"/db_info.conf

    if [ "$force_database" != "" ]
    then
        DATABASE="$force_database"
    fi

    for f in ${1}/*.sql
    do
        if [ ! -f "$f" ]; then break; fi

        echo "Process file $f" | tee -a "$TMP_FILE"
        echo "" | tee -a "$TMP_FILE"

        if [ "$quiet" = "0" ]; then
            head -n1 "$f" | tee -a "$TMP_FILE"
            echo "" | tee -a "$TMP_FILE"
        else
            head -n1 "$f"
            echo ""
        fi

        if [ "$quiet" = "0" ]
        then
            grep "\-\- \-\-" "$f" | tee -a "$TMP_FILE"
        fi
        # Interactive mode: ASK
        process_file=1
        if [ "$interactive" = "1" ]
        then
            process_file=0
            echo "Shall I process this file? (y/n)"
            read line
            if [ "$line" = "y" ]
            then
                process_file=1
            fi
        fi

        # DO the actual work
        if [ "$process_file" = "1" ]
        then
            cat "$TMP_FILE" >> "$log_file"
            echo "Output:" | tee -a "$log_file"
            $MYSQL -u$DB_USER -p$DB_PASS $DATABASE < "$f" > "$TMP_FILE" 2>&1
            if [ "$?" != "0" ]
            then
                cat "$TMP_FILE" | tee -a "$log_file"
                exit 1
            fi
            cat "$TMP_FILE" | tee -a "$log_file"

            echo "" | tee -a "$log_file"
            echo "" | tee -a "$log_file"
        fi

        rm "$TMP_FILE"
    done

    echo "" | tee -a "$log_file"
    echo "" | tee -a "$log_file"
    echo "" | tee -a "$log_file"
}

if [ "$only_directory" = "" ]
then
    OLD_IFS="$IFS"
    IFS=$'\n'
    for dir in `find "$SCRIPT_DIR" -type d`
    do
        #exclude self
        if [ "$dir" != "$SCRIPT_DIR" ]
        then
            if [ "$interactive" = "1" ]
            then
                echo "Shall I process directory $dir? (y/n)"
                read line
                if [ "$line" = "y" ]
                then
                    work_directory "$dir"
                fi
            else
                # work it
                work_directory "$dir"
            fi
        fi
    done
    IFS=$OLD_IFS
else
    if [ -d "$SCRIPT_DIR/$only_directory" ]
    then
        work_directory "$SCRIPT_DIR/$only_directory"
    else
        echo "ERROR - passed parameter of -d option must be a subdirectory (without additional path)"
        exit 1
    fi
fi
