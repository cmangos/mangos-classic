from disco.bot import Plugin
import pymysql.cursors
import socket

# commands marked with the 'protected' decorator will only be processed in the channel specified in the config file
def protected(func):
    def func_wrapper(*args, **kwargs):
        return func(*args, **kwargs) if str(args[1].channel.name) == args[0].client.config.channel else None
    return func_wrapper
    
class AnticheatPlugin(Plugin):
    def load_realms(self):
        self.realms = dict()
        
        print 'ANTICHEAT: Loading realms...'

        with self.db_realm.cursor() as cursor:
            cursor.execute("SELECT id,name FROM realmlist")
            
            for realm in cursor:
                self.realms[realm[0]] = realm[1]
                print 'ANTICHEAT: Found realm #%d: %s' % (realm[0], realm[1])
                
        print 'ANTICHEAT: Done loading realms.  Found %d' % len(self.realms)

    def load(self, ctx):
        super(AnticheatPlugin, self).load(ctx)
        
        self.db_log = pymysql.connect(
            host=self.client.config.database['logs']['host'],
            port=self.client.config.database['logs']['port'],
            user=self.client.config.database['logs']['user'],
            passwd=self.client.config.database['logs']['pass'],
            db=self.client.config.database['logs']['name'])
            
        print 'ANTICHEAT: Connected to log database.'
        
        self.db_realm = pymysql.connect(
            host=self.client.config.database['realm']['host'],
            port=self.client.config.database['realm']['port'],
            user=self.client.config.database['realm']['user'],
            passwd=self.client.config.database['realm']['pass'],
            db=self.client.config.database['realm']['name'])
            
        print 'ANTICHEAT: Connected to realm database.'
        
        self.load_realms()

        self.ra = dict()
        
        for r in self.client.config.realm:
            if r not in self.realms.values():
                raise Exception('Remote administrator credentials given for unrecognized realm %s' % r)
                
            i = self.client.config.realm[r]
            
            s = socket.socket()
            s.connect((i['host'], i['port']))
            
            do_ra_auth(s, i['user'], i['pass'])
            
            self.ra[r] = s
            print 'ANTICHEAT: Remote administration authenticated for realm "%s" successfully.' % r
            
        with self.db_log.cursor() as cursor:
            cursor.execute("SELECT MAX(id) FROM logs_anticheat")
            row = cursor.fetchone()
            self.first_id_anticheat = self.curr_id_anticheat = int(row[0]) if row[0] else 0
            
        print 'ANTICHEAT: First anticheat detection ID: %d' % self.first_id_anticheat
        
        with self.db_log.cursor() as cursor:
            cursor.execute("SELECT MAX(id) FROM logs_spamdetect")
            row = cursor.fetchone()
            self.first_id_antispam = self.curr_id_antispam = int(row[0]) if row[0] else 0
                
        print 'ANTICHEAT: First antispam detection ID: %d' % self.first_id_antispam
        
    def get_account_name(self, id):
        assert self.db_realm.open
        
        with self.db_realm.cursor() as cursor:
            cursor.execute("SELECT username FROM account WHERE id = %d" % id)
            return str(cursor.fetchone()[0])
        
        
    def get_channel(self):
        for chan in self.client.state.channels:
            chan = self.client.state.channels.get(chan)
            
            if chan.name == self.client.config.channel:
                return chan
            
        return None
        
    def report_cheat(self, chan, row):
        assert chan
        
        id = int(row[0])
        time = row[1]
        realm = int(row[2])
        account = int(row[3])
        ip = row[4]
        player = row[5]
        info = row[6]
    
        account_name = self.get_account_name(account)
    
        chan.send_message(
"""
**CHEAT:**
```
Time: %s
Realm: %s (%d)
Account: %s (%d)
IP: %s
Player: %s
Info: %s
Detection ID: %d
```
""" % (
        time.strftime('%x %X'),
        self.realms[realm], realm,
        account_name, account,
        ip,
        player,
        info,
        id))
        
    def report_spam(self, chan, row):
        assert chan
        
        time = row[0]
        realm = int(row[1])
        account = int(row[2])
        fromGuid = int(row[3])
        message = row[5]
        reason = row[6]
        quantity = int(row[7])
        
        account_name = self.get_account_name(account)
        
        chan.send_message(
"""
**SPAM:**
```
Time: %s
Realm: %s (%d)
Account: %s (%d)
From GUID: %d
Message: %s
Reason: %s
Quantity: %d
```
""" % (
        time.strftime('%x %X'),
        self.realms[realm], realm,
        account_name, account,
        fromGuid,
        message,
        reason,
        quantity
        ))
        
    @Plugin.schedule(5, init=False)
    def anticheat_sched(self):
        if not self.db_log.open:
            return
            
        chan = self.get_channel()
        
        # if for whatever reason we don't see the appropriate channel, don't bother querying anything
        if not chan:
            return
                        
        with self.db_log.cursor() as cursor:
            cursor.execute("SELECT id,time,realm,account,ip,player,info FROM logs_anticheat WHERE id > %d ORDER BY id ASC" % self.curr_id_anticheat)
            
            for row in cursor:
                self.curr_id_anticheat = int(row[0])
                self.report_cheat(chan, row)
            
            # this should happen automatically when the cursor goes out of scope, but it doesnt seem to...
            self.db_log.commit()

    @Plugin.schedule(20, init=False)
    def antispam_sched(self):
        if not self.db_log.open:
            return
            
        chan = self.get_channel()
        
        # if for whatever reason we don't see the appropriate channel, don't bother querying anything
        if not chan:
            return
                        
        with self.db_log.cursor() as cursor:
            cursor.execute("SELECT time,realm,accountId,fromGuid,toGuid,message,reason,COUNT(message),MAX(id) " \
                           "FROM logs_spamdetect WHERE id > %d GROUP BY message ORDER BY id ASC" % self.curr_id_antispam)

            for row in cursor:
                if int(row[8]) > self.curr_id_antispam:
                    self.curr_id_antispam = int(row[8])
                self.report_spam(chan, row)
                
            # this should happen automatically when the cursor goes out of scope, but it doesnt seem to...
            self.db_log.commit()
                        
    def is_valid_anticheat_id(self, id):
        return id > self.first_id_anticheat and id <= self.curr_id_anticheat

    def is_valid_antispam_id(self, id):
        return id > self.first_id_antispam and id <= self.curr_id_antispam
        
    @Plugin.command('help')
    @protected
    def command_help(self, event):
        event.msg.reply(
"""
**HELP:**
```
!help                           -- shows this help output (duh?)
!kick <anticheat id>            -- disconnect the world session associated with the given anticheat id
!ban <anticheat id> [seconds]   -- bans the account associated with the given anticheat id, optionally for the given number of seconds
!ipban <anticheat id> [seconds] -- bans the ip associated with the given anticheat id, optionally for the given number of seconds
!stats [account id]             -- shows statistics, optionally specific to the given account id
!mute <account id> [seconds]    -- mute the given account id, optionally for the given number of seconds
!reloadrealms                   -- reload realm list
```
"""
        )
        
    @Plugin.command('reloadrealms')
    @protected
    def command_reloadrealms(self, event):
        self.load_realms()
        event.msg.reply('Loaded `%d` realm(s)' % len(self.realms))
        
    def execute_ra_command(self, realm, command):
        print 'sending command: %s' % command
        self.ra[self.realms[realm]].send("%s\n" % command)
        
    @Plugin.command('kick', '<id:int>')
    @protected
    def command_kick(self, event, id):
        if not self.is_valid_anticheat_id(id):
            event.msg.reply('**Error**: Anticheat ID `%d` not reported recently.  Ignoring request.' % id)
            return
            
        with self.db_log.cursor() as cursor:
            cursor.execute("SELECT realm,player FROM logs_anticheat WHERE id = %d" % id)
            
            row = cursor.fetchone()
            
            # send command to the appropriate realm
            self.execute_ra_command(row[0], ".kick %s" % row[1])
            
            event.msg.reply('Player `%s` kicked.' % row[1])
            
    @Plugin.command('ban', '<id:int> [seconds:int]')
    @protected
    def command_ban(self, event, id, seconds=0):
        if not self.is_valid_anticheat_id(id):
            event.msg.reply('**Error**: Anticheat ID `%d` not reported recently.  Ignoring request.' % id)
            return
            
        with self.db_log.cursor() as cursor:
            cursor.execute("SELECT realm,account,info FROM logs_anticheat WHERE id = %d" % id)
            
            row = cursor.fetchone()
            
            username = self.get_account_name(row[1])
            reason = row[2].replace('"', '')
            
            # send command to the appropriate realm
            self.execute_ra_command(row[0], '.ban account %s %d "%s"' % (username, seconds, reason))
            
            event.msg.reply('Account `%s` (`%d`) banned.  Reason: `%s`' % (username, row[1], reason))
            
    @Plugin.command('ipban', '<id:int> [seconds:int]')
    @protected
    def command_ipban(self, event, id, seconds=0):
        if not self.is_valid_anticheat_id(id):
            event.msg.reply('**Error**: Anticheat ID `%d` not reported recently.  Ignoring request.' % id)
            return
            
        with self.db_log.cursor() as cursor:
            cursor.execute("SELECT realm,ip,info FROM logs_anticheat WHERE id = %d" % id)
            
            row = cursor.fetchone()
            
            reason = row[2].replace('"', '')
            
            # send command to the appropriate realm
            self.execute_ra_command(row[0], '.ban ip %s %d "%s"' % (row[1], seconds, reason))
            
            event.msg.reply('IP `%s` banned.  Reason: `%s`' % (row[1], reason))
            
    @Plugin.command('stats', '[id:int]')
    @protected
    def command_stats(self, event, id=0):
        with self.db_log.cursor() as cursor:
            cursor.execute("SELECT MIN(`time`),CURRENT_TIMESTAMP(),COUNT(*) FROM logs_anticheat")
            
            r = cursor.fetchone()
            
            span = r[1] - r[0]
            total = int(r[2])
            
            hours = span.total_seconds() / 3600.0
            
            total_avg = total / hours
            
            event.msg.reply('**Total**: `%d` detections over `%d` days.  Average `%.3f` detections per hour.' % (
                total, span.days, total_avg))
                
            if id > 0:
                cursor.execute("SELECT MIN(`time`),MAX(`time`),COUNT(*) FROM logs_anticheat WHERE account = " \
                               "(SELECT account FROM logs_anticheat WHERE id = %d)" % id)
                               
                r = cursor.fetchone()
                
                # just in case that anticheat entry is not found
                if not r:
                    return
                    
                span = r[1] - r[0]
                total = int(r[2])
                
                hours = span.total_seconds() / 3600.0
                
                avg = total / hours
        
                event.msg.reply('**Account**: `%d` detections over `%d` days.  Average `%.3f` detections per hour.  %s average.' % (
                    total, span.days, avg, 'Below' if avg <= total_avg else '**Above**'))
                    
    @Plugin.command('mute', '<id:int> [seconds:int]')
    @protected
    def command_mute(self, event, id, seconds=0):
        if not self.is_valid_antispam_id(id):
            event.msg.reply('**Error**: Spam ID `%d` not reported recently.  Ignoring request.' % id)
            return

        # we want to issue the command via RA for it to take effect, but we have to guess as to which realm to do this on.
        # a good guess would be the realm on which spam from this account was most recently detected.
        with self.db_log.cursor() as cursor:
            cursor.execute("SELECT realm FROM logs_spamdetect WHERE accountId = %d ORDER BY `time` DESC LIMIT 1" % id)
            self.execute_ra_command(cursor.fetchone()[0], '.anticheat mute %d %d' % (id, seconds))
            
            log = 'Account `%s` (`%d`) muted.' % (self.get_account_name(id), id)
            
            if seconds > 0:
                log += '  Duration: %d seconds.' % seconds
                
            event.msg.reply(log)
        
        print 'mute %d sec = %d' % (id, seconds)
        
def do_ra_auth(sock, user, password):
    buff = ''
    
    while True:
        c = sock.recv(1)
        
        buff += c
                
        if 'Username: ' in buff:
            sock.send('%s\n' % user)
            buff = ''
        elif 'Password: ' in buff:
            sock.send('%s\n' % password)
            
            result = sock.recv(1)
            
            if result == '+':
                break
                
            raise Exception('RA authentication failed')
