# Branch Predictor Simulator

This project implements a branch predictor simulator to evaluate different branch prediction strategies. The simulator supports three predictor types: Always Taken, Always Not Taken, and Global.

## Usage

### Building the Simulator

Compile the simulator using the provided Makefile:

```bash
make clean
make
```


## Predictor Types

### Always Taken
Always predicts that branches will be taken.

### Always Not Taken
Always predicts that branches will not be taken.

### Global Predictor
Uses a global history register to make predictions based on the recent branch history.

## Output

For each trace file and predictor type, the simulator will output:
- Total branches
- Predictions
- Correct predictions
- Incorrect predictions
- Misprediction rate

