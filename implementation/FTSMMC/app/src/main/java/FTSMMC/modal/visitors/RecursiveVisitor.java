package mmc.modal.visitors;

import mmc.modal.formulas.*;

public abstract class RecursiveVisitor implements FormulaVisitor {

    @Override
    public void visit(BoxFormula formula) {
        formula.getFormula().accept(this);
    }

    @Override
    public void visit(DiamondFormula formula) {
        formula.getFormula().accept(this);
    }

    @Override
    public void visit(LiteralFalse formula) {
        // Do nothing
    }

    @Override
    public void visit(LiteralTrue formula) {
        // Do nothing
    }

    @Override
    public void visit(LogicAndFormula formula) {
        formula.getLeft().accept(this);
        formula.getRight().accept(this);
    }

    @Override
    public void visit(LogicOrFormula formula) {
        formula.getLeft().accept(this);
        formula.getRight().accept(this);
    }

    @Override
    public void visit(MuFormula formula) {
        formula.getRecursionVariable().accept(this);
        formula.getFormula().accept(this);
    }

    @Override
    public void visit(NuFormula formula) {
        formula.getRecursionVariable().accept(this);
        formula.getFormula().accept(this);
    }

    @Override
    public void visit(RecursionVariable formula) {
        // Do nothing
    }
}
