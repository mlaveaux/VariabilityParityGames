package FTSMMC.modal.visitors;

import FTSMMC.modal.formulas.*;

public interface FormulaVisitor {
    void visit(BoxFormula formula);

    void visit(DiamondFormula formula);

    void visit(LiteralFalse formula);

    void visit(LiteralTrue formula);

    void visit(LogicAndFormula formula);

    void visit(LogicOrFormula formula);

    void visit(MuFormula formula);

    void visit(NuFormula formula);

    void visit(RecursionVariable formula);
}
