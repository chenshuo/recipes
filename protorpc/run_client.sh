#!/bin/sh
java -ea -server -Djava.ext.dirs=lib -cp bin echo.EchoClient $1
