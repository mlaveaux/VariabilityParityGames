package FTSMMC.modal.visitors;

import FTSMMC.modal.formulas.*;

public class Counter extends RecursiveVisitor {
    public int counter = 0;

    @Override
    public void visit(BoxFormula formula) {
        counter++;
        super.visit(formula);
    }

    @Override
    public void visit(DiamondFormula formula) {
        counter++;
        super.visit(formula);
    }

    @Override
    public void visit(LiteralFalse formula) {
        counter++;
        super.visit(formula);
    }

    @Override
    public void visit(LiteralTrue formula) {
        counter++;
        super.visit(formula);
    }

    @Override
    public void visit(LogicAndFormula formula) {
        counter++;
        super.visit(formula);
    }

    @Override
    public void visit(LogicOrFormula formula) {
        counter++;
        super.visit(formula);
    }

    @Override
    public void visit(MuFormula formula) {
        counter++;
        super.visit(formula);
    }

    @Override
    public void visit(NuFormula formula) {
        counter++;
        super.visit(formula);
    }

    @Override
    public void visit(RecursionVariable formula) {
        counter++;
        super.visit(formula);
    }
}
