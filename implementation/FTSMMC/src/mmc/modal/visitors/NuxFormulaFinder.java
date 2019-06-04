package mmc.modal.visitors;

import mmc.modal.formulas.FixedPointFormula;
import mmc.modal.formulas.NuFormula;
import mmc.modal.formulas.RecursionVariable;

import java.util.ArrayList;
import java.util.List;

public class NuxFormulaFinder extends RecursiveVisitor implements FormulaVisitor {
    private final RecursionVariable recursionVariable;
    private final List<NuFormula> nuFormulas;

    private NuxFormulaFinder(RecursionVariable recursionVariable) {
        this.recursionVariable = recursionVariable;
        this.nuFormulas = new ArrayList<>();
    }

    public static List<NuFormula> findNuxFormulas(FixedPointFormula formula) {
        NuxFormulaFinder nuxFormulaFinder = new NuxFormulaFinder(formula.getRecursionVariable());
        formula.accept(nuxFormulaFinder);

        return nuxFormulaFinder.nuFormulas;
    }

    @Override
    public void visit(NuFormula formula) {
        if (formula.getVariableMatcher().getFree().contains(this.recursionVariable)) {
            this.nuFormulas.add(formula);
        }
        super.visit(formula);
    }
}
