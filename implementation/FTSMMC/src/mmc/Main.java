package mmc;

import mmc.features.FeatureDiagram;
import mmc.aldebaran.LtsBuilder;
import mmc.aldebaran.SyntaxException;
import mmc.modal.visitors.EmersonLeiAlgorithm;
import mmc.modal.visitors.FormulaCalculator;
import mmc.modal.visitors.NaiveAlgorithm;
import mmc.modal.formulas.Formula;
import mmc.modal.ModalParser;
import mmc.modal.ParseException;
import mmc.models.Lts;
import mmc.models.State;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.Set;
import java.util.stream.Collectors;

public class Main {

    public static void main(String[] args) {
        try {
            FeatureDiagram FD = FeatureDiagram.FeatureDiagramFromBDD(new String[] {"C","Ct","Cp","M","Ma","Mq","L","Ll","Ln","Lh"},
                    "node(C, node(Ct, node(M, node(Ma, node(L, tt, ff), node(Mq, node(L, tt, ff), ff)), node(Ma, ff, node(Mq, ff, node(L, tt, ff)))), node(Cp, node(M, node(Ma, node(L, tt, ff), node(Mq, node(L, tt, ff), ff)), node(Ma, ff, node(Mq, ff, node(L, tt, ff)))), ff)), node(Ct, ff, node(Cp, ff, node(M, node(Ma, node(L, tt, ff), node(Mq, node(L, tt, ff), ff)), node(Ma, ff, node(Mq, ff, node(L, tt, ff)))))))"
            );
            FeatureDiagram.PrimaryFD = FD;
        } catch (Exception e) {
            e.printStackTrace();
        }

        if(args.length != 3) {
            help();
            return;
        }
        Lts lts = loadAldebaranLts(args[1]);
        Formula formula = loadFormula(args[2]);
        FormulaCalculator formulaCalculator = null;
        switch (args[0].toLowerCase()) {
            case "project":
                formulaCalculator = new NaiveAlgorithm(lts);
                break;
            case "vpg":
                formulaCalculator = new EmersonLeiAlgorithm(lts);
                break;
            default:
                help();
                return;
        }

        Set<State> states = formulaCalculator.calculate(formula);
        Set<Integer> result = states.stream()
                .map(State::getNumber)
                .collect(Collectors.toSet());

        System.out.println(formula);
        System.out.println(String.format("Nesting depth: %s", formula.getNestingDepth()));
        System.out.println(String.format("Alternation depth: %s", formula.getAlternationDepth()));
        System.out.println(String.format("Dependent alternation depth: %s", formula.getDependentAlternationDepth()));
        System.out.println(result);
        System.out.println("Verdict: " + String.valueOf(result.contains(0)));
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
