#!/bin/bash
# Build Verification Script for V2X Blockchain Implementation
# This script helps verify that the NS-3 build is successful

echo "=========================================="
echo "V2X Blockchain Build Verification"
echo "=========================================="
echo ""

# Check if we're in NS-3 root directory
if [ ! -f "ns3" ]; then
    echo "❌ Error: Not in NS-3 root directory"
    echo "   Please run this script from NS-3 root (where ./ns3 script is located)"
    exit 1
fi

echo "✅ NS-3 directory detected"
echo ""

# Check if v2x-engine module exists
if [ ! -d "src/v2x-engine" ]; then
    echo "❌ Error: v2x-engine module not found in src/"
    echo "   Please copy the v2x-engine directory to NS-3 src/"
    exit 1
fi

echo "✅ v2x-engine module found"
echo ""

# Check for required header files
REQUIRED_HEADERS=(
    "src/v2x-engine/model/consensus-module.h"
    "src/v2x-engine/model/pow-module.h"
    "src/v2x-engine/model/pos-module.h"
    "src/v2x-engine/model/fba-module.h"
    "src/v2x-engine/model/node-app.h"
    "src/v2x-engine/helper/vanet-engine-helper.h"
)

MISSING_FILES=0
for header in "${REQUIRED_HEADERS[@]}"; do
    if [ ! -f "$header" ]; then
        echo "❌ Missing: $header"
        MISSING_FILES=1
    fi
done

if [ $MISSING_FILES -eq 0 ]; then
    echo "✅ All required header files present"
else
    echo "❌ Some header files are missing"
    exit 1
fi
echo ""

# Check for simulation scripts
if [ ! -f "scratch/v2x-urban-sim.cc" ] || [ ! -f "scratch/v2x-highway-sim.cc" ]; then
    echo "⚠️  Warning: Simulation scripts not found in scratch/"
    echo "   Copy them with: cp src/v2x-engine/scratch/*.cc scratch/"
else
    echo "✅ Simulation scripts found in scratch/"
fi
echo ""

# Try to configure NS-3
echo "Configuring NS-3..."
./ns3 configure --enable-examples > /tmp/ns3-configure.log 2>&1

if [ $? -eq 0 ]; then
    echo "✅ NS-3 configuration successful"
else
    echo "❌ NS-3 configuration failed"
    echo "   Check /tmp/ns3-configure.log for details"
    exit 1
fi
echo ""

# Try to build
echo "Building NS-3 (this may take a few minutes)..."
./ns3 build > /tmp/ns3-build.log 2>&1

if [ $? -eq 0 ]; then
    echo "✅ NS-3 build successful"
else
    echo "❌ NS-3 build failed"
    echo "   Check /tmp/ns3-build.log for details"
    exit 1
fi
echo ""

# Check if simulation binaries exist
if [ -f "scratch/v2x-urban-sim.cc" ]; then
    echo "Verifying v2x-urban-sim..."
    ./ns3 run "v2x-urban-sim --help" > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "✅ v2x-urban-sim is ready to run"
    else
        echo "⚠️  v2x-urban-sim build may have issues"
    fi
fi

if [ -f "scratch/v2x-highway-sim.cc" ]; then
    echo "Verifying v2x-highway-sim..."
    ./ns3 run "v2x-highway-sim --help" > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "✅ v2x-highway-sim is ready to run"
    else
        echo "⚠️  v2x-highway-sim build may have issues"
    fi
fi
echo ""

# Check Python dependencies
echo "Checking Python environment..."
if command -v python3 &> /dev/null; then
    echo "✅ Python 3 found: $(python3 --version)"
    
    # Check for required packages
    python3 -c "import pandas" 2>/dev/null
    if [ $? -eq 0 ]; then
        echo "✅ pandas installed"
    else
        echo "⚠️  pandas not installed (pip install pandas)"
    fi
    
    python3 -c "import matplotlib" 2>/dev/null
    if [ $? -eq 0 ]; then
        echo "✅ matplotlib installed"
    else
        echo "⚠️  matplotlib not installed (pip install matplotlib)"
    fi
    
    python3 -c "import numpy" 2>/dev/null
    if [ $? -eq 0 ]; then
        echo "✅ numpy installed"
    else
        echo "⚠️  numpy not installed (pip install numpy)"
    fi
else
    echo "⚠️  Python 3 not found"
fi
echo ""

echo "=========================================="
echo "Build Verification Complete!"
echo "=========================================="
echo ""
echo "Next steps:"
echo "1. Run urban simulation: ./ns3 run v2x-urban-sim"
echo "2. Run highway simulation: ./ns3 run v2x-highway-sim"
echo "3. Analyze results: cd src/v2x-engine/analysis && python analyze_results.py ../../v2x-urban-metrics.csv"
echo ""
echo "For more information, see:"
echo "- README.md for detailed documentation"
echo "- QUICKSTART.md for quick setup guide"
echo "- TECHNICAL_SPEC.md for architecture details"
echo ""
