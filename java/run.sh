#!/bin/sh

CLASSPATH=lib/junit-4.8.2.jar:\
lib/joda-time-1.6.2.jar:\
lib/groovy-1.7.10.jar:\
lib/asm-3.2.jar:\
lib/antlr-2.7.7.jar:\
./bin

export CLASSPATH
mkdir bin
javac -d bin billing/*.java billing/test/*.java
java -ea org.junit.runner.JUnitCore billing.test.VipCustomerTest billing.test.NormalCustomerTest
