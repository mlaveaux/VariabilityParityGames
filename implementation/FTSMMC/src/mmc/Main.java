package mmc;

import mmc.SVPG.Edge;
import mmc.SVPG.SVPG;
import mmc.SVPG.Vertex;
import mmc.features.FeatureDiagram;
import mmc.aldebaran.LtsBuilder;
import mmc.aldebaran.SyntaxException;
import mmc.modal.visitors.*;
import mmc.modal.formulas.Formula;
import mmc.modal.ModalParser;
import mmc.modal.ParseException;
import mmc.models.Lts;

import java.io.IOException;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Random;

public class Main {

    public static void main(String[] args) {
        Lts fts;
        switch (args[0].toLowerCase()) {
            case "project":
                loadFeatureDiagram(args[1]);
                fts = loadAldebaranLts(args[2]);
                projectToLts(fts,args[3]);
                break;
            case "vpg":
                loadFeatureDiagram(args[1]);
                fts = loadAldebaranLts(args[2]);
                Formula formula = loadFormula(args[3]);
                createSVPG(fts, formula, args[4]);
                break;
            case "parseaction":
                Formula f = loadFormula(args[1]);
                FormulaWriter fw = new FormulaWriter(parseActionList(args[2]));
                f.accept(fw);
                System.out.println(fw.getStringValue());
                break;
            case "randomgame":
                randomSVPG(
                        Integer.parseInt(args[1]),
                        Integer.parseInt(args[2]),
                        Integer.parseInt(args[3]),
                        Integer.parseInt(args[4]),
                        Integer.parseInt(args[5]),
                        Float.parseFloat(args[6]),
                        args[7].charAt(0),
                        args[8]
                );
                break;
            default:
                help();
                return;
        }
    }

    private static void randomSVPG(int n, int p, int l, int h, int c, float lambda, char typeofrandom, String directory){
        String[] features = new String[c];
        for(int i = 0;i<c;i++){
            features[i] = String.valueOf(i);
        }
        try {
            FeatureDiagram.PrimaryFD = FeatureDiagram.FeatureDiagramFromBDD(features,"tt");
        } catch (Exception e) {
            e.printStackTrace();
        }
        Random r = new Random();
        Vertex[] vertices = new Vertex[n];
        for(int i = 0;i<n;i++){
            vertices[i] = new Vertex();
            vertices[i].owner = r.nextInt(2);
            vertices[i].prio = r.nextInt(p+1);
        }
        for(int i = 0;i<n;i++){
            int m = r.nextInt(h-l+1) + l;
            int conf = FeatureDiagram.PrimaryFD.getZero();
            boolean[] targeted = new boolean[n];
            Arrays.fill(targeted, false);
            for(int j = 0;j<m;j++){
                Edge e = new Edge();
                int t = r.nextInt(n);
                e.target = vertices[t];
                if(targeted[t])
                    continue;
                targeted[t] = true;
                if(typeofrandom == 'F')
                    e.configurations = FeatureDiagram.PrimaryFD.getFeaturedRandomConfigurations(lambda);
                else
                    e.configurations = FeatureDiagram.PrimaryFD.getRandomConfigurations(lambda);
                conf = FeatureDiagram.PrimaryFD.or(conf, e.configurations);
                vertices[i].addEdge(e);
            }
        }
        SVPG s = new SVPG(vertices);
        s.makeInfinite();
        for(int i = 0;i<FeatureDiagram.PrimaryFD.products.size();i++)
        {
            int product = FeatureDiagram.PrimaryFD.products.get(i);
            String productString = FeatureDiagram.PrimaryFD.productStrings.get(i);
            String proj = s.projectToPG(product);
            try {
                Files.write(Paths.get(directory, "sSVPG"+productString),
                        proj.getBytes(Charset.forName("UTF-8")),
                        StandardOpenOption.CREATE);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        try {
            Files.write(Paths.get(directory, "SVPG"),
                    s.toSVPG().getBytes(Charset.forName("UTF-8")),
                    StandardOpenOption.CREATE);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private static void createSVPG(Lts fts, Formula formula, String directory)
    {
        Counter c = new Counter();
        formula.accept(c);
        System.out.println("Formula size: " + String.valueOf(c.counter));
        SVPG svpg = new SVPG(c.counter * fts.getStates().length + 2);
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
                Files.write(Paths.get(directory, "sSVPG"+productString),
                        a,
                        Charset.forName("UTF-8"));
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        try {
            Files.write(Paths.get(directory, "SVPG"),
                    svpg.toSVPG().getBytes(Charset.forName("UTF-8")),
                    StandardOpenOption.CREATE);
        } catch (IOException e) {
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

    private static String[] parseActionList(String filename){
        String actionlistfile = readFile(filename);
        return actionlistfile.split(",");
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
            e.printStackTrace();
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
            e.printStackTrace();
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
