package mmc.modal.visitors;

import mmc.modal.formulas.FixedPointFormula;
import mmc.modal.formulas.MuFormula;

import java.util.ArrayList;
import java.util.List;

public class MuFormulaFinder extends RecursiveVisitor implements FormulaVisitor {
    private final List<MuFormula> muFormulas;
    private MuFormulaFinder() {
        this.muFormulas = new ArrayList<>();
    }

    public static List<MuFormula> findMuFormulas(FixedPointFormula formula) {
        MuFormulaFinder muFormulaFinder = new MuFormulaFinder();
        formula.accept(muFormulaFinder);

        return muFormulaFinder.muFormulas;
    }

    @Override
    public void visit(MuFormula formula) {
        this.muFormulas.add(formula);
        super.visit(formula);
    }
}
