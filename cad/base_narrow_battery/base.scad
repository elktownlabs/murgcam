WIDTH = 98.0;
LENGTH = 111;
HEIGHT = 3.5;
HOLDER_HEIGHT = 1.7;
BORDER_HEIGHT = 13;
HOLDER_WIDTH = 5.5;
BORDER_WIDTH=0;
BORDER_WIDTH_FRONTBACK = 4;

union() {
    cube([WIDTH, LENGTH, HOLDER_HEIGHT]);
    translate([HOLDER_WIDTH, 0, HOLDER_HEIGHT/2])
        difference() {
            cube([WIDTH - (2*HOLDER_WIDTH), LENGTH, BORDER_HEIGHT]);
            translate([BORDER_WIDTH, BORDER_WIDTH_FRONTBACK, HEIGHT]) 
                cube([WIDTH - (2*(HOLDER_WIDTH+BORDER_WIDTH)), LENGTH-(2*BORDER_WIDTH_FRONTBACK), BORDER_HEIGHT-HEIGHT]);
        }
    translate([0, WIDTH/2+5, 0]) linear_extrude(height=HOLDER_HEIGHT) circle(r=10);
}

