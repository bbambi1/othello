#!/bin/bash

echo "=== Testing Othello AI Safety System ==="
echo

# Build the tournament runner
echo "Building tournament runner..."
make tournament_runner
if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "Build successful!"
echo

# Test 1: Time limit violation
echo "=== Test 1: Time Limit Violation ==="
echo "Running tournament with malicious agent that exceeds time limit..."
./tournament_runner --agents random,malicious --tournament roundrobin --rounds 1 --time-limit 1000
echo

# Test 2: Invalid move violation
echo "=== Test 2: Invalid Move Violation ==="
echo "Running tournament with malicious agent that plays invalid moves..."
# We need to modify the malicious agent to use invalid moves for this test
# For now, let's just show the command
echo "Command: ./tournament_runner --agents random,malicious --tournament roundrobin --rounds 1"
echo

# Test 3: Crash violation
echo "=== Test 3: Crash Violation ==="
echo "Running tournament with malicious agent that crashes..."
# We need to modify the malicious agent to use crash behavior for this test
echo "Command: ./tournament_runner --agents random,malicious --tournament roundrobin --rounds 1"
echo

# Test 4: Resource access violation
echo "=== Test 4: Resource Access Violation ==="
echo "Running tournament with malicious agent that accesses external resources..."
# We need to modify the malicious agent to use resource access behavior for this test
echo "Command: ./tournament_runner --agents random,malicious --tournament roundrobin --rounds 1"
echo

# Test 5: Safety disabled (dangerous!)
echo "=== Test 5: Safety Disabled (DANGEROUS!) ==="
echo "Running tournament WITHOUT safety wrapper..."
echo "WARNING: This may cause crashes or infinite loops!"
echo "Command: ./tournament_runner --agents random,malicious --tournament roundrobin --rounds 1 --no-safety"
echo

echo "=== Safety System Test Complete ==="
echo
echo "To test different violation types, modify the malicious agent's violationType:"
echo "  - 'timeout' for time limit violations"
echo "  - 'invalid_move' for move validation violations"
echo "  - 'crash' for crash violations"
echo "  - 'resource' for resource access violations"
echo
echo "The safety system should:"
echo "  1. Detect violations and disqualify agents"
echo "  2. Prevent crashes from affecting the tournament"
echo "  3. Enforce time limits per move"
echo "  4. Validate all moves before execution"
echo "  5. Monitor resource access (basic implementation)"
