/*
 * First Attempt at generating the TLC output code from an image.
 * Run this with "java AnimationCreator"
 * It will read any image file in the current directory and create an animation for the TLC library.
 *
 * Right now this only works with 1 TLC with 16 LEDS connected to it, where
 * output0 is the bottom and output15 is the top.
 *
 * For best results make your files 16 pixels high and as wide as you want.  Each vertical pixel
 * corresponds to an LED output.
 *
 * Alex Leone <acleone ~AT~ gmail.com>, 2008-11-12
 */

import java.util.*;
import java.awt.*;
import java.awt.image.*;
import java.io.*;
import javax.imageio.*;

public class AnimationCreator {

    public static void main(String[] args) throws IOException {
        if (args.length == 0) {
            autoProcess();
        }
    }

    public static void autoProcess() throws IOException {
        File currentDirectory = new File (".");
        File[] files = currentDirectory.listFiles();
        int animationCount = 1;
        for (File file : files) {
            if (!file.isFile())
                continue;
            String fileName = file.getName();
            String suffix = fileName.substring(fileName.indexOf('.') + 1);
            if(!canReadFormat(suffix))
                continue;
            String baseName = fileName.substring(0, fileName.indexOf('.'));
            String varName = "ani_" + baseName.toLowerCase();
            String outputName = varName + ".h";
            System.out.println("Writing " + outputName);
            BufferedImage image = ImageIO.read(file);
            PrintStream output = new PrintStream(new File(outputName));
            output.println("#define    " + varName.toUpperCase() + "_FRAMES    " + image.getWidth());
            output.println("uint8_t " + varName + "[NUM_TLCS * 24 * " + varName.toUpperCase() + "_FRAMES] PROGMEM = {");
            int[] rowRGB = new int[16];
            for (int w = 0; w < image.getWidth(); w++) {
                for (int h = 0; h < 16; h++) {
                    rowRGB[h] = image.getRGB(w, 15 - h);
                }
                parseRow(rowRGB, output);
            }
            output.println("};");
            System.out.println("Wrote " + image.getWidth() + " frames to " + outputName);
            animationCount++;
        }
    }

    // Returns true if the specified format name can be read
    public static boolean canReadFormat(String formatName) {
        Iterator<ImageReader> iter = ImageIO.getImageReadersByFormatName(formatName);
        return iter.hasNext();
    }

    public static double rgbToGrayscaleIntensity(int rgb) {
        Color c = new Color(rgb);
        return 0.2989 * c.getRed() + 0.5870 * c.getGreen() + 0.1140 * c.getBlue();
    }

    public static void parseRow(int[] rowRGB, PrintStream output) {
        output.print("\t");
        for (int i = rowRGB.length - 1; i >= 0; i -= 2) {
            int a = (255 - (int)Math.round(rgbToGrayscaleIntensity(rowRGB[i])));
            int b = (255 - (int)Math.round(rgbToGrayscaleIntensity(rowRGB[i - 1])));
            output.print(((a >> 4) & 0xFF) + "," + (((a << 4) | (b >> 8)) & 0xFF) + "," + (b & 0xFF) + ",");
             //System.out.print(
            //        "GS_DUO(" + (255 - Math.round(rgbToGrayscaleIntensity(rowRGB[i]))) + "," +
            //        (255 - Math.round(rgbToGrayscaleIntensity(rowRGB[i - 1]))) + "),");
        }
        output.println();
    }


}