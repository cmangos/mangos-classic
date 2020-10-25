// Item Instance conversion for cMaNGOS
// Author: Henhouse
// Based on C++ project: https://github.com/vmangos/ItemInstance
package main

import (
	"bytes"
	"database/sql"
	"fmt"
	"io/ioutil"
	"log"
	"strconv"
	"strings"
	"time"

	_ "github.com/go-sql-driver/mysql"
)

const EntriesPerInsert = 25000

func main() {
	fmt.Println("\n\n########################################################")
	fmt.Println("# BACKUP YOUR DATABASE BEFORE RUNNING THIS SCRIPT!     #")
	fmt.Println("#    If you are migrating existing cmangos data you    #")
	fmt.Println("#    *MUST* perform this migration BEFORE applying     #")
	fmt.Println("#    the characters db SQL update script!              #")
	fmt.Println("########################################################")
	fmt.Println("#  This script will take your existing item_instance   #")
	fmt.Println("#  data and convert it to the new format. Then, run    #")
	fmt.Println("#  the cmangos character db update script as normal.   #")
	fmt.Println("#  Lastly, run the output script generated from here.  #")
	fmt.Println("########################################################")
	fmt.Println()

	PromptExpansionSelection()

	// Prompt user for DB info and connect.
	db := HandleConnectDB()

	// Count entries to convert so we know what size to make the array.
	log.Println("Counting entries...")
	var totalEntries uint
	row := db.QueryRow("SELECT COUNT(*) FROM `item_instance`")
	if err := row.Scan(&totalEntries); err != nil {
		log.Println(err)
	}

	if totalEntries == 0 {
		log.Println("No entries in `item_instance`. Exiting.")
		return
	}
	log.Printf("Counted %v entries.\n\n", totalEntries)

	log.Println("Loading all item instance entries...")
	var query string
	switch chosenExp {
	case ExpWotLK:
		query = "SELECT `guid`, `data`, `text` FROM `item_instance`"
	default:
		query = "SELECT `data` FROM `item_instance`"
	}
	rows, err := db.Query(query)
	if err != nil {
		log.Println(err)
	}

	// Create slice with capacity for all entries.
	blobResults := make([]string, 0, totalEntries)
	texts := make([]string, 0, totalEntries)

	var data string
	var text sql.NullString
	var guid uint32

	for rows.Next() {
		switch chosenExp {
		case ExpWotLK:
			err = rows.Scan(&guid, &data, &text)
		default:
			err = rows.Scan(&data)
		}
		if err != nil {
			log.Println(err)
		}

		blobResults = append(blobResults, data)
		if chosenExp == ExpWotLK {
			texts = append(texts, text.String)
		}
	}
	log.Printf("Loaded.\n\n")

	parseStart := time.Now()
	log.Println("Beginning parse... this may take a minute or two.")

	// Way faster than concatenating strings.
	var entireQuery strings.Builder
	for i := range blobResults {
		if i%EntriesPerInsert == 0 {
			entireQuery.WriteString("INSERT INTO `item_instance` VALUES \n")
		}

		blob := blobResults[i]
		text := ""

		if chosenExp == ExpWotLK {
			text = texts[i]
		}

		entireQuery.WriteString(ParseDataBlob(blob, text))

		if i == len(blobResults)-1 || (i > 0 && (i+1)%EntriesPerInsert == 0) {
			entireQuery.WriteString(";\n")
		} else {
			entireQuery.WriteString(",")
		}
		entireQuery.WriteString("\n")
	}
	db.Close()

	timeElapsed := time.Since(parseStart)
	log.Printf("Done parsing in %v\n\n", timeElapsed.String())

	log.Println("Writing full query to file: item_instance_converted.sql")

	queryAsBytes := bytes.NewBufferString(entireQuery.String()).Bytes()
	if err = ioutil.WriteFile("item_instance_converted.sql", queryAsBytes, 0644); err != nil {
		log.Fatal("Failed to write file! Check permissions.")
	}

	log.Println("Done.")
}

func ParseDataBlob(blob, text string) string {
	values := strings.Split(blob, " ")
	var itemGuid = values[fieldIndex(fieldItemGuid)]
	var itemEntry = values[fieldIndex(fieldItemEntry)]
	var ownerGuid = uint64(stringToUint32(values[fieldIndex(fieldOwnerGuid)]))
	var creatorGuid = uint64(stringToUint32(values[fieldIndex(fieldCreatorGuid)]))
	var giftCreator = uint64(stringToUint32(values[fieldIndex(fieldGiftCreator)]))
	var stackCount = values[fieldIndex(fieldStackCount)]
	var duration = values[fieldIndex(fieldDuration)]

	var spellCharges string
	for i := fieldIndex(fieldSpellChargesStart); i <= fieldIndex(fieldSpellChargesEnd); i++ {
		if i != fieldIndex(fieldSpellChargesStart) {
			spellCharges += " "
		}

		// Stored as uint32 but we need to cast to int32.
		spellCharges += fmt.Sprintf("%d", int32(stringToUint32(values[i])))
	}

	var flags = values[fieldIndex(fieldFlags)]

	var enchantments string
	for i := fieldIndex(fieldEnchantmentsStart); i <= fieldIndex(fieldEnchantmentsEnd); i++ {
		if i != fieldIndex(fieldEnchantmentsStart) {
			enchantments += " "
		}

		// Use stringValues here since they're already as strings
		enchantments += values[i]
	}

	var randomPropertyId = int32(stringToUint32(values[fieldIndex(fieldRandomPropertyId)])) // Stored as uint32, but we want int32 representation.
	var durability = values[fieldIndex(fieldDurability)]

	var lastField string // Either textId or playedTime depending on expansion.
	switch chosenExp {
	case ExpVanilla:
		fallthrough
	case ExpTBC:
		lastField = values[fieldIndex(fieldTextId)]
	case ExpWotLK:
		lastField = values[fieldIndex(fieldPlayedTime)] + ", \"" + text + "\""
	}

	return " (" +
		itemGuid + ", " +
		fmt.Sprintf("%d", ownerGuid) + ", " +
		itemEntry + ", " +
		fmt.Sprintf("%d", creatorGuid) + ", " +
		fmt.Sprintf("%d", giftCreator) + ", " +
		stackCount + ", " +
		duration + ", " +
		"'" + spellCharges + "'" + ", " +
		flags + ", " +
		"'" + enchantments + "'" + ", " +
		fmt.Sprintf("%d", randomPropertyId) + ", " +
		durability + ", " +
		lastField +
		")"
}

func stringToUint32(str string) uint32 {
	result, err := strconv.ParseUint(str, 10, 32)
	if err != nil {
		log.Println(err)
	}

	return uint32(result)
}

func PromptExpansionSelection() {
	fmt.Println("Choose expansion for migration (type 1, 2, or 3 and press Enter):")
	fmt.Println(" 1. Vanilla (mangos-classic)")
	fmt.Println(" 2. TBC (mangos-tbc)")
	fmt.Println(" 3. WotLK (mangos-wotlk)")
	fmt.Print("> ")

	fmt.Scanf("%d", &chosenExp)

	switch chosenExp {
	case ExpVanilla:
	case ExpTBC:
	case ExpWotLK:
	default:
		log.Fatal("Invalid expansion type specified. Please specify between 1 and 3.")
	}
	fmt.Println("---------------------------------")
	fmt.Println()
}

func HandleConnectDB() *sql.DB {
	var host, port, user, pass, database string

	fmt.Println("Database connection credentials:")
	fmt.Println("Host:")
	fmt.Scanf("%s", &host)

	fmt.Println("Port:")
	fmt.Scanf("%s", &port)

	fmt.Println("User:")
	fmt.Scanf("%s", &user)

	fmt.Println("Password:")
	fmt.Scanf("%s", &pass)

	fmt.Println("Database:")
	fmt.Scanf("%s", &database)

	fmt.Println()
	log.Println("Connecting to database...")

	var connStr string
	connStr += user
	connStr += ":" + pass
	connStr += "@tcp(" + host + ":" + port + ")"
	connStr += "/" + database

	session, err := sql.Open("mysql", connStr)
	if err != nil {
		log.Fatal("Couldn't connect to DB: ", err)
	}

	// sql.Open above does not return connection failure, so we must ping to ensure the session is valid.
	err = session.Ping()
	if err != nil {
		log.Fatal("Couldn't connect to DB:", err)
	}

	log.Println("Database connection established.\n")

	return session
}

type expansion uint8

const (
	ExpVanilla expansion = 1
	ExpTBC               = 2
	ExpWotLK             = 3
)

type field uint8

// Item field references (changes depending on expansion).
const (
	fieldItemGuid field = iota
	fieldItemEntry
	fieldOwnerGuid
	fieldCreatorGuid
	fieldGiftCreator
	fieldStackCount
	fieldDuration
	fieldSpellChargesStart
	fieldSpellChargesEnd
	fieldFlags
	fieldEnchantmentsStart
	fieldEnchantmentsEnd
	fieldRandomPropertyId
	fieldTextId
	fieldDurability
	fieldPlayedTime // WotLK only
)

var fieldExpMapper = map[expansion]map[field]uint8{
	ExpVanilla: {
		fieldItemGuid:          0,
		fieldItemEntry:         3,
		fieldOwnerGuid:         6,
		fieldCreatorGuid:       10,
		fieldGiftCreator:       12,
		fieldStackCount:        14,
		fieldDuration:          15,
		fieldSpellChargesStart: 16,
		fieldSpellChargesEnd:   20,
		fieldFlags:             21,
		fieldEnchantmentsStart: 22,
		fieldEnchantmentsEnd:   42,
		fieldRandomPropertyId:  44,
		fieldTextId:            45,
		fieldDurability:        46,
	},
	ExpTBC: {
		fieldItemGuid:          0,
		fieldItemEntry:         3,
		fieldOwnerGuid:         6,
		fieldCreatorGuid:       10,
		fieldGiftCreator:       12,
		fieldStackCount:        14,
		fieldDuration:          15,
		fieldSpellChargesStart: 16,
		fieldSpellChargesEnd:   20,
		fieldFlags:             21,
		fieldEnchantmentsStart: 22,
		fieldEnchantmentsEnd:   54,
		fieldRandomPropertyId:  56,
		fieldTextId:            57,
		fieldDurability:        58,
	},
	ExpWotLK: {
		fieldItemGuid:          0,
		fieldItemEntry:         3,
		fieldOwnerGuid:         6,
		fieldCreatorGuid:       10,
		fieldGiftCreator:       12,
		fieldStackCount:        14,
		fieldDuration:          15,
		fieldSpellChargesStart: 16,
		fieldSpellChargesEnd:   20,
		fieldFlags:             21,
		fieldEnchantmentsStart: 22,
		fieldEnchantmentsEnd:   57,
		fieldRandomPropertyId:  59,
		fieldDurability:        60,
		fieldPlayedTime:        62,
	},
}

// Provided by user.
var chosenExp expansion

// fieldIndex returns the item field index for the chosen expansion.
func fieldIndex(field field) uint8 {
	return fieldExpMapper[chosenExp][field]
}
