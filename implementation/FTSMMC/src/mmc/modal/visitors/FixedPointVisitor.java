package mmc.modal.visitors;

import mmc.modal.formulas.MuFormula;
import mmc.modal.formulas.NuFormula;

public class FixedPointVisitor extends RecursiveVisitor implements FormulaVisitor {
    @Override
    public void visit(MuFormula formula) {
        formula.findVariables();
    }

    @Override
    public void visit(NuFormula formula) {
        formula.findVariables();
    }
}
