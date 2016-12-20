# This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

Copy the content of this directory into your client directory.
Those scripts need some binaries to works.
Make sure there is in this folder all required binaries.
- ad(.exe)              sources in contrib/extractor        dbc/maps extractor
- vmapExtractor(.exe)   sources in contrib/vmap_extractor   VMAPS extractor
- vmap_assembler(.exe)  sources in contrib/vmap_assembler   VMAPS builder
- MoveMapGen(.exe)      sources in contrib/mmap             MMAPS biulder

If you dont have them, you have to build them each one by one.

Then just run ExtractResources.sh
On Windows, you can run the file within the "Git bash" (which should be shipped with your Git software)
by invoking "sh ExtractResources.sh"

This file will ask you what you want to extract.

You can chose from the following:
* Extract DBCs/ maps (required for MaNGOS to work)
* Extract vmaps (expected for MaNGOS to work)
* Extract mmaps (optional, and will require very long time to create)
* Update mmaps with data from offmesh.txt (suggested after updates of this file)

In case you want to extract mmaps you will be asked how many processes should be used for mmap-extraction;
Recommanded is to use the number of CPUs

By default the scripts will create log files
MaNGOSExtractor.log for overall progress of the extraction, and
MaNGOSExtractor_detailed.log which includes all the information about the extraction.

If you want to toggle some parts in the extraction process, there are a few internal variables in the extraction scripts, that can be modified.

Consider the Readme's in the directories extractor, vmap_assembler, vmap_extractor and mmap for further information about detail.

Also especially related to mmaps updating you might be interested in using the MoveMapGen.sh script.
