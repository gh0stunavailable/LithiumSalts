#include "ClockMenu.h"
#include "core/display.h"
#include "core/settings.h"

void ClockMenu::optionsMenu() {
    runClockLoop();
}

void ClockMenu::drawIcon(float scale) {
    clearIconArea();

    int radius = scale * 30;
    int pointerSize = scale * 15;

    // Case
    tft.drawArc(
        iconCenterX,
        iconCenterY,
        1.1*radius, radius,
        0, 360,
        LithiumSaltsConfig.priColor,
        LithiumSaltsConfig.bgColor
    );

    // Pivot center
    tft.fillCircle(iconCenterX, iconCenterY, radius/10, LithiumSaltsConfig.priColor);

    // Hours & minutes
    tft.drawLine(
        iconCenterX, iconCenterY,
        iconCenterX - 2*pointerSize/3, iconCenterY - 2*pointerSize/3,
        LithiumSaltsConfig.priColor
    );
    tft.drawLine(
        iconCenterX, iconCenterY,
        iconCenterX + pointerSize, iconCenterY - pointerSize,
        LithiumSaltsConfig.priColor
    );
}