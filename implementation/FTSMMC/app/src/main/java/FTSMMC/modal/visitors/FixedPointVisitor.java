package FTSMMC.modal.visitors;

import FTSMMC.modal.formulas.MuFormula;
import FTSMMC.modal.formulas.NuFormula;

public class FixedPointVisitor extends RecursiveVisitor {
    @Override
    public void visit(MuFormula formula) {
        formula.findVariables();
    }

    @Override
    public void visit(NuFormula formula) {
        formula.findVariables();
    }
}
