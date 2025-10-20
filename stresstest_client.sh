#!/bin/bash
# Experiment 1: Many clients joining a bare server. (./launch_bare_server + ./stresstest_client)
# Experiment 2: Many clients joining a server with 100 entities 
# Usage: ./stresstest_client

${BUILDPATH}/GameClient > log.client1 &
${BUILDPATH}/GameClient > log.client2 &
${BUILDPATH}/GameClient > log.client3 &
${BUILDPATH}/GameClient > log.client4 &
${BUILDPATH}/GameClient > log.client5 &
${BUILDPATH}/GameClient > log.client6 &
${BUILDPATH}/GameClient > log.client7 &
${BUILDPATH}/GameClient > log.client8 &
${BUILDPATH}/GameClient > log.client9 &
