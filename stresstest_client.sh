#!/bin/bash
# Experiment 1: Many clients joining a bare server.

# Usage: CLIENT=./path/to/GameClient ./tresstest_client
${CLIENT} > log.client1 &
${CLIENT} > log.client2 &
${CLIENT} > log.client3 &
${CLIENT} > log.client4 &
${CLIENT} > log.client5 &
${CLIENT} > log.client6 &
${CLIENT} > log.client7 &
${CLIENT} > log.client8 &
${CLIENT} > log.client9 &
