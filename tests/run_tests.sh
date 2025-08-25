#!/bin/bash

# wmswitch test suite
# Tests core functionality and config generation

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
WMSWITCH="$PROJECT_ROOT/bin/wmswitch"
TEST_CONFIG_DIR="/tmp/wmswitch_tests"

echo "🧪 wmswitch Test Suite"
echo "======================"

# Clean up previous test runs
rm -rf "$TEST_CONFIG_DIR"
mkdir -p "$TEST_CONFIG_DIR"/{i3,hypr,aerospace}

echo "✅ Test environment prepared"

# Test 1: Version command
echo "📋 Test 1: Version command"
if $WMSWITCH version | grep -q "wmswitch version"; then
    echo "✅ Version command works"
else
    echo "❌ Version command failed"
    exit 1
fi

# Test 2: Help command
echo "📋 Test 2: Help command"
if $WMSWITCH --help | grep -q "Usage:"; then
    echo "✅ Help command works"
else
    echo "❌ Help command failed"
    exit 1
fi

# Test 3: Config validation
echo "📋 Test 3: Config validation"
if $WMSWITCH validate "$PROJECT_ROOT/examples/basic_config.toml" > /dev/null 2>&1; then
    echo "✅ Config validation works"
else
    echo "❌ Config validation failed"
    exit 1
fi

# Test 4: Dry-run generation
echo "📋 Test 4: Dry-run config generation"
if $WMSWITCH --dry-run generate "$PROJECT_ROOT/examples/basic_config.toml" > /dev/null 2>&1; then
    echo "✅ Dry-run generation works"
else
    echo "❌ Dry-run generation failed"
    exit 1
fi

# Test 5: Actual config generation
echo "📋 Test 5: Actual config generation"
export XDG_CONFIG_HOME="$TEST_CONFIG_DIR"
if $WMSWITCH generate "$PROJECT_ROOT/examples/basic_config.toml" > /dev/null 2>&1; then
    echo "✅ Config generation works"
else
    echo "❌ Config generation failed"
    exit 1
fi

# Test 6: Verify generated files exist and have content
echo "📋 Test 6: Verify generated configs"
for config_file in "$TEST_CONFIG_DIR/i3/config" "$TEST_CONFIG_DIR/hypr/hyprland.conf" "$TEST_CONFIG_DIR/aerospace/aerospace.toml"; do
    if [[ -f "$config_file" && -s "$config_file" ]]; then
        echo "✅ Generated config exists: $(basename "$(dirname "$config_file")")"
    else
        echo "❌ Missing or empty config: $config_file"
        exit 1
    fi
done

# Test 7: Verify i3 config content
echo "📋 Test 7: Verify i3 config content"
i3_config="$TEST_CONFIG_DIR/i3/config"
if grep -q "workspace.*dev.*output main" "$i3_config" && \
   grep -q "bindsym \$mod+1 workspace" "$i3_config" && \
   grep -q "bindsym \$mod+Shift+1 move container" "$i3_config"; then
    echo "✅ i3 config content is correct"
else
    echo "❌ i3 config content is incorrect"
    exit 1
fi

# Test 8: Verify Hyprland config content  
echo "📋 Test 8: Verify Hyprland config content"
hypr_config="$TEST_CONFIG_DIR/hypr/hyprland.conf"
if grep -q "workspace = 1, monitor:main" "$hypr_config" && \
   grep -q "bind = \$mainMod 1, workspace, 1" "$hypr_config" && \
   grep -q "bind = \$mainMod SHIFT 1, movetoworkspace, 1" "$hypr_config"; then
    echo "✅ Hyprland config content is correct"
else
    echo "❌ Hyprland config content is incorrect"
    exit 1
fi

# Test 9: Test invalid config handling
echo "📋 Test 9: Invalid config handling"
echo "invalid toml content" > "$TEST_CONFIG_DIR/invalid.toml"
if ! $WMSWITCH validate "$TEST_CONFIG_DIR/invalid.toml" > /dev/null 2>&1; then
    echo "✅ Invalid config properly rejected"
else
    echo "❌ Invalid config was accepted"
    exit 1
fi

# Test 10: Test backup functionality
echo "📋 Test 10: Backup functionality"
echo "# existing config" > "$TEST_CONFIG_DIR/i3/config"
export XDG_CONFIG_HOME="$TEST_CONFIG_DIR"
$WMSWITCH --backup generate "$PROJECT_ROOT/examples/basic_config.toml" > /dev/null 2>&1
if [[ -f "$TEST_CONFIG_DIR/i3/config.wmswitch.backup" ]]; then
    echo "✅ Backup functionality works"
else
    echo "❌ Backup was not created"
    exit 1
fi

echo ""
echo "🎉 All tests passed!"
echo "wmswitch is working correctly!"

# Clean up
rm -rf "$TEST_CONFIG_DIR"