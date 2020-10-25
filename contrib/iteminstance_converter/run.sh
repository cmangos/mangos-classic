#!/bin/bash

clear

# Ensure Go is installed on the machine and usable
if ! [ -x "$(command -v go)" ]; then
  echo "Error: go is not installed. Aborting" >&2
  exit 1
fi


echo "Retrieving go dependencies..."
go mod vendor
echo "Dependencies vendored."

echo "Beginning conversion script..."
go run main.go

echo ""
