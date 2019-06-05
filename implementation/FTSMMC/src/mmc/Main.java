package mmc;

import mmc.SVPG.SVPG;
import mmc.features.FeatureDiagram;
import mmc.aldebaran.LtsBuilder;
import mmc.aldebaran.SyntaxException;
import mmc.modal.visitors.CreateSVPG;
import mmc.modal.visitors.EmersonLeiAlgorithm;
import mmc.modal.visitors.FormulaCalculator;
import mmc.modal.visitors.NaiveAlgorithm;
import mmc.modal.formulas.Formula;
import mmc.modal.ModalParser;
import mmc.modal.ParseException;
import mmc.models.Lts;
import mmc.models.State;

import java.awt.*;
import java.io.FileWriter;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Set;
import java.util.stream.Collectors;

public class Main {

    public static void main(String[] args) {
        loadFeatureDiagram(args[1]);
        Lts fts = loadAldebaranLts(args[2]);
        switch (args[0].toLowerCase()) {
            case "project":
                projectToLts(fts,args[3]);
                break;
            case "vpg":
                Formula formula = loadFormula(args[3]);
                createSVPG(fts, formula, args[4]);
                break;
            default:
                help();
                return;
        }
    }

    private static void createSVPG(Lts fts, Formula formula, String directory)
    {
        SVPG svpg = new SVPG();
        formula.accept(new CreateSVPG(svpg, fts, fts.getStart(),null));
        svpg.makeInfinite();
        for(int i = 0;i<FeatureDiagram.PrimaryFD.products.size();i++)
        {
            int product = FeatureDiagram.PrimaryFD.products.get(i);
            String productString = FeatureDiagram.PrimaryFD.productStrings.get(i);
            String proj = svpg.projectToPG(product);
            ArrayList<String> a = new ArrayList<String>();
            a.add(proj);
            try {
                Files.write(Paths.get(directory, productString),
                        a,
                        Charset.forName("UTF-8"));
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        try {

            ArrayList<String> a = new ArrayList<String>();
            a.add(svpg.toSVPG());
            try {
                Files.write(Paths.get(directory, "SVPG"),
                        a,
                        Charset.forName("UTF-8"));
            } catch (IOException e) {
                e.printStackTrace();
            }
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
    }

    private static void projectToLts(Lts fts, String directory)
    {
        for(int i = 0;i< FeatureDiagram.PrimaryFD.products.size();i++)
        {
            int product = FeatureDiagram.PrimaryFD.products.get(i);
            String productString = FeatureDiagram.PrimaryFD.productStrings.get(i);

            String proj = fts.projectToAlberant(product);
            ArrayList<String> a = new ArrayList<String>();
            a.add(proj);

            try {
                Files.write(Paths.get(directory, productString + ".aut"),
                        a,
                        Charset.forName("UTF-8"));
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    private static void loadFeatureDiagram(String filename)
    {
        String fdfile = readFile(filename);
        try {
            FeatureDiagram FD = FeatureDiagram.FeatureDiagramFromBDD(fdfile.split("\n")[0].split(","),
                    fdfile.split("\n")[1]
            );
            FeatureDiagram.PrimaryFD = FD;
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    private static Lts loadAldebaranLts(String filename) {
        // http://www.mcrl2.org/web/user_manual/language_reference/lts.html#id1
        String aldebaran = readFile(filename);
        LtsBuilder lb = new LtsBuilder();
        try {
            return lb.buildStrict(aldebaran);
        } catch (SyntaxException e) {
            System.err.println(String.format("Unable to parse aldebaran lts from: %s", filename));
            System.exit(2);
            return null;
        }
    }

    private static Formula loadFormula(String filename) {
        String formulaText = readFile(filename);
        ModalParser mp = new ModalParser(formulaText);
        try {
            return mp.parse();
        } catch (ParseException e) {
            System.err.println(String.format("Unable to parse formula from: %s", filename));
            System.err.print(e);
            System.exit(2);
            return null;
        }
    }

    private static String readFile(String filename) {
        try {
            return new String(Files.readAllBytes(Paths.get(filename)));
        } catch (IOException e) {
            System.err.println(String.format("Unable to read input file: %s", filename));
            System.exit(2);
            return null;
        }
    }

    private static void help() {
        System.err.println("3 arguments required: (project | VPG) <fts file> <modal formula file>");
        System.exit(1);
    }
}