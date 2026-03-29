; Test script for shell color tools
echo "Testing shprintf..."
shprintf "This is red text" COLOR RED
shprintf "This is green text" COLOR GREEN
shprintf "This is blue text" COLOR BLUE

echo "Testing shsetcolor and shresetcolor..."
shsetcolor COLOR YELLOW
echo "This should be yellow"
shresetcolor
echo "This should be back to default"

echo "Testing background colors with shprintf..."
shprintf "White text on red background" COLOR BG_RED
shresetcolor
