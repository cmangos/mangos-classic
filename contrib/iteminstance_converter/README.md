# Item Instance data converter
Tool to convert the datablob in the item_instance table to extracted format.
https://github.com/henhouse/ItemInstanceConverter

## Instructions & Usage

##### Warning:
This script should be run **BEFORE** applying the character-db SQL update. This tool will extract
the current data inside the `item_instance` table and prepare it in the new format. It will output `INSERT`
statements for each item to be put back in after you apply the cmangos character-db update, which will `TRUNCATE`
the table.

BACKUP YOUR CHARACTER DATABASE!

##### Usage:
- Download and install the latest version of [Go from Google](https://golang.org/dl).
     - Win10 users with [Scoop](https://scoop.sh) can instead use: `scoop install go`
     - macOS users with [Homebrew](https://brew.sh) can instead use: `brew install go`
 - Run `run.sh` inside this folder a bash terminal (Linux/macOS) or Powershell (Windows) and follow the steps.

## Credit
Inspired from: https://github.com/vmangos/ItemInstance
