package mmc.modal.visitors;

import mmc.modal.formulas.FixedPointFormula;
import mmc.modal.formulas.NuFormula;

import java.util.ArrayList;
import java.util.List;

public class NuFormulaFinder extends RecursiveVisitor implements FormulaVisitor {
    private final List<NuFormula> nuFormulas;
    private NuFormulaFinder() {
        this.nuFormulas = new ArrayList<>();
    }

    public static List<NuFormula> findNuFormulas(FixedPointFormula formula) {
        NuFormulaFinder nuFormulaFinder = new NuFormulaFinder();
        formula.accept(nuFormulaFinder);

        return nuFormulaFinder.nuFormulas;
    }

    @Override
    public void visit(NuFormula formula) {
        this.nuFormulas.add(formula);
        super.visit(formula);
    }
}
