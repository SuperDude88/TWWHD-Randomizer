#include "mainwindow.h"
#include "../ui_mainwindow.h"

#include <QColorDialog>
#include <QImage>

#include <utility/file.hpp>
#include <utility/color.hpp>

std::tuple<std::string, std::string> MainWindow::get_option_name_and_color_name_from_sender_object_name() {
    std::string objectName = this->sender()->objectName().toStdString();
    std::string optionName = "";

    if (objectName.ends_with("_hex_code_input")) {
        optionName = objectName.substr(0, objectName.length() - strlen("_hex_code_input"));
    } else if (objectName.ends_with("_randomize_color")) {
        optionName = objectName.substr(0, objectName.length() - strlen("_randomize_color"));
    } else if (objectName.ends_with("_reset_color")) {
        optionName = objectName.substr(0, objectName.length() - strlen("_reset_color"));
    }

    auto colorName = optionName.substr(strlen("custom_color_"));
    return {optionName, colorName};
}

void MainWindow::initialize_color_presets_list() {
    ui->custom_color_preset->addItem("Default");
    ui->custom_color_preset->addItem("Custom");

    // Get data from color_presets.yaml
    std::string presetDataStr;
    Utility::getFileContents(DATA_PATH "logic/data/color_presets.yaml", presetDataStr, true);
    YAML::Node presetDataTree = YAML::Load(presetDataStr);

    // Loop through and add each preset
    for (const auto& presetObject : presetDataTree) {
        auto presetName = presetObject["Name"].as<std::string>();
        ui->custom_color_preset->addItem(QString::fromStdString(presetName));

        for (auto type : {"hero", "casual"}) {
            for (const auto& presetColor : presetObject["Colors"][type]) {
                auto optionName = presetColor.first.as<std::string>();
                auto optionColor = presetColor.second.as<std::string>();

                presetColors[presetName][type][optionName] = optionColor;
            }

            // Add in defaults for any unspecified options
            auto defaultColors = DefaultColors::getDefaultColorsMap(!strcmp(type, "casual"));
            for (auto& [option, color] : defaultColors) {
                if (!presetColors[presetName][type].contains(option)) {
                    presetColors[presetName][type][option] = color;
                }
            }
        }
    }

    // Add in default preset also
    for (auto type : {"hero", "casual"}) {
        auto defaultColors = DefaultColors::getDefaultColorsMap(!strcmp(type, "casual"));
        for (auto& [optionName, color] : defaultColors) {
            presetColors["Default"][type][optionName] = color;
        }
    }
}

void MainWindow::initialize_mask_pixels() {
    // Store the x and y of each pixel that each maskfile modifies
    // and lookup the pixels later when making preview modifications
    for (auto type : {"hero", "casual"}) {
        auto defaultColors = DefaultColors::getDefaultColorsMap(!strcmp(type, "casual"));
        for (auto& [optionName, color] : defaultColors) {
            auto maskImage = QImage(DATA_PATH + QString("assets/link preview/preview_") + type + "_" + QString::fromStdString(optionName) + ".png");
            for (int x = 0; x < maskImage.width(); x++) {
                for (int y = 0; y < maskImage.height(); y++) {
                    if (maskImage.pixelColor(x, y).red() == 255 && maskImage.pixelColor(x, y).blue() == 0) {
                        maskPixels[type][optionName].emplace_back(x, y);
                    }
                }
            }
        }
    }
}

void MainWindow::set_color(const std::string& optionName, std::string color, const bool& updatePreview, const bool& saveColorAsCustom) {
    auto colorName = optionName.substr(std::string("custom_color_").length());

    auto colorButton = customColorSelectorButtons[optionName];
    auto hexInput = customColorHexCodeInputs[optionName];
    auto resetButton = customColorResetButtons[optionName];

    auto colorRGB = hexColorStrToRGB(color);

    for (auto& c : color) {
        c = std::toupper(c);
    }

    // Set hex line edit with new color (without triggering slot)
    hexInput->blockSignals(true);
    hexInput->setText(QString::fromStdString(color));
    hexInput->blockSignals(false);

    // Determine if color selector text needs to be black or white to properly
    // contrast with the new background color
    auto colorHSV = RGBToHSV(colorRGB.R, colorRGB.G, colorRGB.B);
    std::string textColor = "";
    if (colorHSV.V > 0.5) {
        textColor = "rgb(0, 0, 0)";
    } else {
        textColor = "rgb(255, 255, 255)";
    }

    std::string r = std::to_string(int(colorRGB.R * 255));
    std::string g = std::to_string(int(colorRGB.G * 255));
    std::string b = std::to_string(int(colorRGB.B * 255));

    colorButton->setStyleSheet(QString::fromStdString(
        "background-color: rgb(" + r + ", " + g + ", " + b + ");"
        "color: " + textColor + ";"
    ));

    auto defaultColors = config.settings.player_in_casual_clothes ? DefaultColors::casualColors : DefaultColors::heroColors;
    std::string defaultColor = "";
    for (auto& [name, color] : defaultColors) {
        if (name == colorName) {
            defaultColor = color;
            break;
        }
    }

    // Hide reset button if color is default
    if (color == defaultColor) {
        resetButton->setVisible(false);
    } else {
        resetButton->setVisible(true);
    }

    // Get previous custom colors to compare against later
    auto previousCustomColors = config.settings.custom_colors;
    config.settings.custom_colors[colorName] = color;

    // Check if the current color combination is equal
    // to any presets and set the preset (without triggering the slot)
    ui->custom_color_preset->blockSignals(true);
    bool foundPreset = false;
    for (auto& [presetName, types] : presetColors) {
        auto& curPreset = types[config.settings.player_in_casual_clothes ? "casual" : "hero"];
        if (config.settings.custom_colors == curPreset) {
            foundPreset = true;
            ui->custom_color_preset->setCurrentText(QString::fromStdString(presetName));
            break;
        }
    }
    if(!foundPreset) {
        ui->custom_color_preset->setCurrentText(QString::fromStdString("Custom"));
    }
    ui->custom_color_preset->blockSignals(false);

    // Update preview if necessary
    if (updatePreview && config.settings.custom_colors != previousCustomColors) {
        update_preview();
    }
}

void MainWindow::open_custom_color_chooser() {
    auto optionName = this->sender()->objectName().toStdString();

    auto colorName = optionName.substr(std::string("custom_color_").length());

    auto hexColor = config.settings.custom_colors[colorName];
    if (!isValidHexColor(hexColor)) {
        return;
    }
    auto colorRGB = hexColorStrToRGB(hexColor);
    auto initialColor = QColor(colorRGB.R * 255, colorRGB.G * 255, colorRGB.B * 255, 255);
    auto color = QColorDialog::getColor(initialColor, this, "Select color");
    if (!color.isValid()) {
        return;
    }

    colorRGB = RGBA<double>(color.redF(), color.greenF(), color.blueF(), 1);
    set_color(optionName, RGBToHexColorStr(colorRGB));
}

bool MainWindow::custom_color_hex_code_changed() {

    auto [optionName, colorName] = get_option_name_and_color_name_from_sender_object_name();

    if (std::string("QLineEdit") != this->sender()->metaObject()->className()) {
        return false;
    }

    auto color = ((QLineEdit*) this->sender())->text().toStdString();
    if (!isValidHexColor(color)) {
        return false;
    }

    set_color(optionName, color);
    return true;
}

void MainWindow::custom_color_hex_code_finished_editing() {
    auto isValidColor = custom_color_hex_code_changed();
    if (!isValidColor) {
        auto [optionName, colorName] = get_option_name_and_color_name_from_sender_object_name();
        set_color(optionName, config.settings.custom_colors[colorName]);
    }
}

std::tuple<int, int> MainWindow::get_random_h_and_v_shifts_for_custom_color(const std::string& hexColor) {
    auto colorRGB = hexColorStrToRGB(hexColor);
    auto colorHSV = RGBToHSV(colorRGB);

    int s = round(colorHSV.S * 100);
    int v = round(colorHSV.V * 100);

    int minVShift = -40;
    int maxVShift = 40;

    if (s < 10) {
        // For very unsaturated colors, we want to limit the range of value
        // randomization to exclude results that wouldn't change anything anyway.
        // This effectively stops white and black from having a 50% chance to not change at all.
        minVShift = std::max(-40, 0-v);
        maxVShift = std::min(40, 100-v);
    }

    auto hShift = rand() % 360;
    auto vShift = (rand() % (maxVShift - minVShift)) + minVShift;

    return {hShift, vShift};
}

void MainWindow::randomize_one_custom_color() {
    auto [optionName, colorName] = get_option_name_and_color_name_from_sender_object_name();

    auto defaultColors = DefaultColors::getDefaultColorsMap(config.settings.player_in_casual_clothes);
    auto defaultColor = defaultColors[colorName];

    auto [hShift, vShift] = get_random_h_and_v_shifts_for_custom_color(defaultColor);
    auto color = HSVShiftColor(defaultColor, hShift, vShift);

    set_color(optionName, color);
}

void MainWindow::reset_one_custom_color() {
    auto [optionName, colorName] = get_option_name_and_color_name_from_sender_object_name();

    auto defaultColors = DefaultColors::getDefaultColorsMap(config.settings.player_in_casual_clothes);
    auto defaultColor = defaultColors[colorName];

    if (config.settings.custom_colors[colorName] != defaultColor) {
        set_color(optionName, defaultColor);
    }
}

void MainWindow::setup_color_options() {
    auto baseColors = config.settings.player_in_casual_clothes ? DefaultColors::casualColors : DefaultColors::heroColors;

    clear_layout(ui->custom_colors_layout);
    customColorSelectorButtons.clear();
    customColorHexCodeInputs.clear();
    customColorResetButtons.clear();

    std::string curPreset = ui->custom_color_preset->currentText().toStdString();
    // If this is our first time running the function, then initialize
    // color presets and mask pixels. (Tried putting this in the main window
    // constructor, but it kept crashing for some reason, so it's here instead)
    // Also read colors from the config into the base colors
    static bool firstTime = true;
    if (firstTime) {
        for (auto& [optionName, defaultColor] : baseColors) {
            if (config.settings.custom_colors.contains(optionName)) {
                defaultColor = config.settings.custom_colors[optionName];
            }
        }
        initialize_color_presets_list();
        initialize_mask_pixels();
        firstTime = false;
    }
    // If we have a specific preset selected, then keep those colors
    else if (curPreset != "Custom") {
        auto colors = presetColors[curPreset][config.settings.player_in_casual_clothes ? "casual" : "hero"];
        for (auto& [optionName, defaultColor] : baseColors) {
            defaultColor = colors[optionName];
        }
    }

    config.settings.custom_colors.clear();

    for (auto& [customColorName, defaultColor] : baseColors) {

        auto optionName = "custom_color_" + customColorName;
        auto colorWidget = new QWidget();
        auto colorLayout = new QHBoxLayout();
        colorLayout->setContentsMargins(0, 3, 0, 3);
        colorWidget->setLayout(colorLayout);

        // Add label
        auto labelForColorSelector = new QLabel();
        labelForColorSelector->setText(QString::fromStdString(customColorName + " Color"));
        colorLayout->addWidget(labelForColorSelector);

        // Color Hex Code Input
        auto colorHexCodeInput = new QLineEdit();
        colorHexCodeInput->setText(QString::fromStdString(defaultColor));
        colorHexCodeInput->setObjectName(QString::fromStdString(optionName + "_hex_code_input"));
        colorHexCodeInput->setFixedWidth(QFontMetrics(QFont()).horizontalAdvance("CCCCCC") + 5);
        colorHexCodeInput->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        colorLayout->addWidget(colorHexCodeInput);

        // Color Randomize
        auto colorRandomizeButton = new QPushButton();
        colorRandomizeButton->setText("Random");
        colorRandomizeButton->setObjectName(QString::fromStdString(optionName +"_randomize_color"));
        colorRandomizeButton->setFixedWidth(QFontMetrics(QFont()).horizontalAdvance("Random") + 11);
        colorLayout->addWidget(colorRandomizeButton);

        // Color Selector
        auto colorSelectorButton = new QPushButton();
        colorSelectorButton->setText(("Click to set color"));
        colorSelectorButton->setObjectName(QString::fromStdString(optionName));
        colorLayout->addWidget(colorSelectorButton);

        // Color Reset
        auto colorResetButton = new QPushButton();
        colorResetButton->setText("X");
        colorResetButton->setObjectName(QString::fromStdString(optionName + "_reset_color"));
        colorResetButton->setFixedWidth(QFontMetrics(QFont()).horizontalAdvance("X") + 11);
        auto sizePolicy = colorResetButton->sizePolicy();
        sizePolicy.setRetainSizeWhenHidden(true);
        colorResetButton->setSizePolicy(sizePolicy);
        colorResetButton->setVisible(false);
        colorLayout->addWidget(colorResetButton);

        customColorSelectorButtons[optionName] = colorSelectorButton;
        customColorHexCodeInputs[optionName] = colorHexCodeInput;
        customColorResetButtons[optionName] = colorResetButton;
        connect(colorSelectorButton, &QPushButton::clicked, this, &MainWindow::open_custom_color_chooser);
        connect(colorHexCodeInput, &QLineEdit::textChanged, this, &MainWindow::custom_color_hex_code_changed);
        connect(colorHexCodeInput, &QLineEdit::editingFinished, this, &MainWindow::custom_color_hex_code_finished_editing);
        connect(colorRandomizeButton, &QPushButton::clicked, this, &MainWindow::randomize_one_custom_color);
        connect(colorResetButton, &QPushButton::clicked, this, &MainWindow::reset_one_custom_color);

        ui->custom_colors_layout->addWidget(colorWidget);
        set_color(optionName, defaultColor, false, false);
    }

    update_preview();
}

void MainWindow::update_preview() {

    auto  defaultColors = DefaultColors::getDefaultColorsMap(config.settings.player_in_casual_clothes);
    auto& currentColors = config.settings.custom_colors;

    auto type = config.settings.player_in_casual_clothes ? "casual" : "hero";

    auto preview = QImage(DATA_PATH + QString("assets/link preview/preview_") + type + ".png");

    for (auto& [optionName, color] : config.settings.custom_colors) {

        auto& defaultColor = defaultColors[optionName];
        auto baseColor16Bit = hexColorStrTo16Bit(defaultColor);
        auto replacementColor16Bit = hexColorStrTo16Bit(color);

        if (color != defaultColor) {

            // Use map to keep track of already exchanged colors
            std::unordered_map<uint16_t, uint16_t> colorReplacements = {};
            for (auto& [x, y] : maskPixels[type][optionName]) {
                auto curPixel = preview.pixelColor(x, y);
                auto curColor16Bit = colorHSVTo16Bit(RGBToHSV(curPixel.redF(), curPixel.greenF(), curPixel.blueF()));

                uint16_t newColor16Bit = 0;
                // Lookup color if its exchange has already been calculated
                if (colorReplacements.contains(curColor16Bit)) {
                    newColor16Bit = colorReplacements[curColor16Bit];
                } else {
                    newColor16Bit = colorExchange(baseColor16Bit, replacementColor16Bit, curColor16Bit);
                    colorReplacements[curColor16Bit] = newColor16Bit;
                }

                curPixel.setRedF(((  newColor16Bit & 0xF800) >> 11) / 31.0f);
                curPixel.setGreenF(((newColor16Bit & 0x07E0) >> 5) / 63.0f);
                curPixel.setBlueF((  newColor16Bit & 0x001F) / 31.0f);

                preview.setPixelColor(x, y, curPixel.convertTo(QColor::Rgb));
            }
        }
    }

    auto pixmap = QPixmap::fromImage(preview);
    pixmap = pixmap.scaledToWidth(220, Qt::SmoothTransformation);

    ui->custom_model_preview_label->setPixmap(pixmap);

}

void MainWindow::on_custom_color_preset_currentIndexChanged(const int &arg1)
{
    auto text = ui->custom_color_preset->currentText();

    if (text == "Custom") {
        return;
    }

    auto& preset = presetColors[text.toStdString()][config.settings.player_in_casual_clothes ? "casual" : "hero"];
    for (auto& [optionName, color] : preset) {
        set_color("custom_color_" + optionName, color, false, false);
    }

    update_preview();
}

void MainWindow::on_randomize_all_custom_colors_together_clicked()
{
    auto defaultColors = DefaultColors::getDefaultColorsMap(config.settings.player_in_casual_clothes);

    int hShift = rand() % 360;
    int vShift = (rand() % 81) - 40;
    for (auto& [customColorName, color] : defaultColors) {
        auto newColor = HSVShiftColor(color, hShift, vShift);

        std::string optionName = "custom_color_" + customColorName;
        set_color(optionName, newColor, false);
    }

    update_preview();
}

void MainWindow::on_randomize_all_custom_colors_separately_clicked()
{
    auto defaultColors = DefaultColors::getDefaultColorsMap(config.settings.player_in_casual_clothes);

    for (auto& [customColorName, color] : defaultColors) {
        auto [hShift, vShift] = get_random_h_and_v_shifts_for_custom_color(color);
        auto newColor = HSVShiftColor(color, hShift, vShift);

        std::string optionName = "custom_color_" + customColorName;
        set_color(optionName, newColor, false);
    }

    update_preview();
}
