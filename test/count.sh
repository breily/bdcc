#!/bin/bash

./run.sh 2> /dev/null | grep pass | wc -l
