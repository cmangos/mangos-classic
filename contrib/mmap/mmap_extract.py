#!/usr/bin/python

"""
This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
"""

import argparse
import glob
import os
import subprocess
import sys
from collections import deque
from threading import Thread
from time import time, sleep
from multiprocessing import cpu_count

# Maps which have no map tiles and must be extracted individually.
MAPS_TO_EXTRACT = [
    '13',  # test
    '34',  # StormwindJail
    '35',  # StormwindPrison
    '42',  # Collin
    '43',  # WailingCaverns
    '44',  # Monastery
    '48',  # Blackfathom
    '70',  # Uldaman
    '90',  # GnomeragonInstance
    '109',  # SunkenTemple
    '229',  # BlackRockSpire
    '230',  # BlackrockDepths
    '249',  # OnyxiaLairInstance
    '349',  # Mauradon
    '369',  # DeeprunTram
    '389',  # OrgrimmarInstance
    '409',  # MoltenCore
    '429',  # DireMaul
    '449',  # AlliancePVPBarracks
    '450'  # HordePVPBarracks
]

def get_tiles():
    os.chdir('maps')
    file_names = glob.glob('*.map')

    tiles = []
    for file_name in file_names:
        # Convert to int and back because the extractor doesn't like leading zeroes.
        map_id = str(int(file_name[0:3]))
        tile_x = str(int(file_name[3:5]))
        tile_y = str(int(file_name[5:7]))
        tiles.append((map_id, tile_x, tile_y))

    os.chdir('..')
    # Remove any tiles that are already done.
    os.chdir('mmaps')
    done_names = glob.glob('*.mmtile')
    os.chdir('..')

    for file_name in done_names:
        map_id = str(int(file_name[0:3]))
        tile_x = str(int(file_name[3:5]))
        tile_y = str(int(file_name[5:7]))
        if (map_id, tile_x, tile_y) in tiles:
            tiles.remove((map_id, tile_x, tile_y))

    return tiles


class WorkerThread(Thread):
    def __init__(self, tile_queue, map_queue, print_queue):
        Thread.__init__(self)
        self.tile_queue = tile_queue
        self.map_queue = map_queue
        self.print_queue = print_queue

    def extract_tile(self, map_id, tile_x, tile_y):
        if sys.platform == 'win32':
            stInfo = subprocess.STARTUPINFO()
            stInfo.dwFlags |= 0x00000001
            stInfo.wShowWindow = 7
            cFlags = subprocess.CREATE_NEW_CONSOLE
            binName = "MoveMapGen.exe"
        else:
            stInfo = None
            cFlags = 0
            binName = "./MoveMapGen"

        start_time = time()

        if tile_x and tile_y:
            print_queue.append('+++ Map: {0} Tile: {1},{2}'.format(map_id, tile_x, tile_y))
            subprocess.call([binName, map_id, "--silent", "--tile", tile_y + ',' + tile_x],
                            startupinfo=stInfo, creationflags=cFlags)
        else:
            print_queue.append('+++ Map: ' + map_id)
            subprocess.call([binName, map_id, "--silent"],
                            startupinfo=stInfo, creationflags=cFlags)

        secs = int(time() - start_time)
        if secs >= 60:
            mins = secs / 60
            secs %= 60
            time_taken = '{0} min {1} sec'.format(mins, secs)
        elif secs >= 1:
            time_taken = str(secs) + ' sec'
        else:
            time_taken = 'skipping'

        if tile_x and tile_y:
            left = len(self.tile_queue)
            print_queue.append('--- Map: {0} Tile: {1},{2} ({3}; {4} left)'
                               .format(map_id, tile_x, tile_y, time_taken, left))
        else:
            left = len(self.map_queue)
            print_queue.append('--- Map: {0} ({1}; {2} left)'
                               .format(map_id, time_taken, left))

    def extract_map(self, map_id):
        self.extract_tile(map_id, None, None)

    def run(self):
        # Run until there are no more tiles or maps in the queues.
        while True:
            try:
                tile = self.tile_queue.popleft()
                map_id = None
            except IndexError:
                try:
                    tile = None
                    map_id = self.map_queue.popleft()
                except IndexError:
                    return

            if tile:
                self.extract_tile(*tile)
            else:
                self.extract_map(map_id)


def make_print_thread(queue, done):
    def print_thread():
        while True:
            # Check for the kill signal.
            if len(done) > 0:
                return

            while True:
                try:
                    print queue.popleft()
                except IndexError:
                    break

            sleep(1)

    return print_thread


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Extract movement maps.')
    parser.add_argument('--threads', '-t', type=int, help='how many worker threads to spawn')

    args = parser.parse_args()
    if args.threads >= 1:
        num_threads = args.threads
    else:
        num_threads = cpu_count() - 1

    remaining_tiles = deque(get_tiles())
    remaining_maps = deque(MAPS_TO_EXTRACT)
    print_queue = deque()
    threads = []

    # Spawn a thread to print messages.
    done_signal = []
    Thread(target=make_print_thread(print_queue, done_signal)).start()

    # Spawn the specified number of threads.
    for i in range(num_threads):
        thread = WorkerThread(remaining_tiles, remaining_maps, print_queue)
        thread.start()
        threads.append(thread)

    # Wait for all threads to exit.
    for thread in threads:
        thread.join()

    # Signal the print thread that everything is done.
    done_signal.append(None)