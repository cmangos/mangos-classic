#!/bin/bash

uname="root"
upasswd="root"
cmd="mysql -u${uname} -p${upasswd} -s"
cmd2="mysql -ucmangos -pcmangos -s"

#创建库
printf "创建库 classicmangos  \n"
${cmd} -e "drop database if exists classicmangos"
${cmd} -e "CREATE DATABASE classicmangos DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci"
printf "创建库 classiccharacters \n"
${cmd} -e "drop database if exists classiccharacters"
${cmd} -e "CREATE DATABASE classiccharacters DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci"
printf "创建库 classicrealmd \n"
${cmd} -e "drop database if exists classicrealmd"
${cmd} -e "CREATE DATABASE classicrealmd DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci"


${cmd} -e "DROP USER cmangos@localhost";

printf "创建用户 cmangos  \n"
${cmd} -e "CREATE USER cmangos@localhost IDENTIFIED BY 'cmangos'"

printf "创建用户权限 SIUDCDAL...\n"
${cmd} -e "GRANT SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, ALTER, LOCK TABLES, CREATE TEMPORARY TABLES, EXECUTE, ALTER ROUTINE, CREATE ROUTINE ON classicmangos.* TO \"cmangos\"@\"localhost\""
${cmd} -e "GRANT SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, ALTER, LOCK TABLES, CREATE TEMPORARY TABLES, EXECUTE ON classiccharacters.* TO \"cmangos\"@\"localhost\""
${cmd} -e "GRANT SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, ALTER, LOCK TABLES, CREATE TEMPORARY TABLES, EXECUTE ON classicrealmd.* TO cmangos@localhost"


#修改缓存大小256M(1024*1024*256),检查方法show global variables like 'max_allowed_packet';
printf "config mysql cache size \n"
${cmd} -e "set global max_allowed_packet=268435456"

#创建基础数据
printf "导入 classicmangos sql/base/mangos.sql \n"
${cmd2} --database=classicmangos  < sql/base/mangos.sql
printf "导入 dbc 相关数据库文件 \n"
for sql_file in `ls sql/base/dbc/original_data/*.sql`; do ${cmd} --database=classicmangos < $sql_file ; done
for sql_file in `ls sql/base/dbc/cmangos_fixes/*.sql`; do ${cmd} --database=classicmangos < $sql_file ; done
#创建角色属性数据
printf "导入 classiccharacters sql/base/characters.sql \n"
${cmd2} classiccharacters < sql/base/characters.sql
#创建登录数据库
printf "导入 classiccharacters sql/base/realmd.sql \n"
${cmd2} classicrealmd < sql/base/realmd.sql
printf "import finish！！！ \n"