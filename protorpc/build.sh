#!/bin/sh
mkdir -p bin
javac -extdirs lib -d bin echo/*.java
