#!/bin/bash

# Build script for participants to create their own AI agents
# This script helps you integrate your agent into the competition framework

echo "=== Othello AI Agent Builder ==="
echo "This script will help you build and test your AI agent"
echo ""

# Check if we're in the right directory
if [ ! -f "Makefile" ]; then
    echo "Error: Please run this script from the project root directory"
    echo "Make sure you see the Makefile in the current directory"
    exit 1
fi

# Check if SFML is available
echo "Checking SFML availability..."
make check-sfml

echo ""
echo "=== Building Your Agent ==="

# Check if the participant has added their agent files
if [ -f "src/my_ai_agent.cpp" ] || [ -f "include/my_ai_agent.h" ]; then
    echo "Found participant agent files!"
    echo "Building with your agent..."
    
    # Build all components
    make clean
    make build-all
    
    if [ $? -eq 0 ]; then
        echo ""
        echo "Build successful! Your agent has been integrated."
        echo ""
        echo "=== Testing Your Agent ==="
        echo "Running a quick test tournament..."
        
        # Test the agent in a simple tournament
        ./tournament_runner --agents random,myfirstagent --tournament roundrobin --rounds 1
        
        echo ""
        echo "=== Next Steps ==="
        echo "1. Test your agent: ./tournament_runner --agents random,myfirstagent --tournament roundrobin"
        echo "2. Play against your agent: ./othello (choose Human vs AI mode)"
        echo "3. Watch your agent play: ./othello (choose AI vs AI mode)"
        echo "4. Run a full tournament: ./tournament_runner --agents random,greedy,minmax,myfirstagent --tournament roundrobin --rounds 3"
        
    else
        echo ""
        echo "Build failed. Please check the error messages above."
        echo "Common issues:"
        echo "- Missing #include statements"
        echo "- Syntax errors in your code"
        echo "- Missing REGISTER_AI_AGENT macro"
        exit 1
    fi
    
else
    echo "No participant agent files found."
    echo ""
    echo "To create your own AI agent:"
    echo "1. Copy examples/my_first_ai_agent.cpp to src/my_ai_agent.cpp"
    echo "2. Copy examples/my_first_ai_agent.h to include/my_ai_agent.h"
    echo "3. Modify the code to implement your strategy"
    echo "4. Run this script again"
    echo ""
    echo "Or use the existing example agent:"
    echo "cp examples/my_first_ai_agent.cpp src/"
    echo "cp examples/my_first_ai_agent.h include/"
    echo ""
    echo "Then run: make build-all"
fi

echo ""
echo "=== Available Commands ==="
echo "make build-all          - Build all components"
echo "make run               - Run console version"
echo "make run-gui           - Run GUI version (if SFML available)"
echo "make run-tournament    - Run tournament runner"
echo "./tournament_runner --help  - Show tournament options"
echo ""
echo "Happy coding!"
