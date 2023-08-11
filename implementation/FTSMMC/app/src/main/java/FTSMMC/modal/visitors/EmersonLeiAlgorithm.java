package mmc.modal.visitors;

import mmc.modal.formulas.*;
import mmc.models.Lts;

import java.util.HashSet;

public class EmersonLeiAlgorithm extends NaiveAlgorithm implements FormulaCalculator, FormulaVisitor {
    private enum Bound {
        MU_BOUNDED,
        NU_BOUNDED,
        UNBOUNDED,
    }

    private Bound bound;

    public EmersonLeiAlgorithm(Lts lts) {
        super(lts);
        this.bound = Bound.UNBOUNDED;
    }

    @Override
    public void visit(MuFormula formula) {
        RecursionVariable recursionVariable = formula.getRecursionVariable();
        Formula subFormula = formula.getFormula();

        if(!this.getFixedPointResults().containsKey(recursionVariable)) {
            this.getFixedPointResults().put(recursionVariable, new HashSet<>());
        }

        if (this.bound == Bound.NU_BOUNDED) {
            for (RecursionVariable boundedVariable: formula.getVariableMatcher().getBounded()) {
                this.getFixedPointResults().put(boundedVariable, new HashSet<>());
            }
        }

        Bound oldBound = this.bound;
        this.bound = Bound.NU_BOUNDED;
        this.putFormulaResult(formula, this.fixedPoint(subFormula, recursionVariable));
        this.bound = oldBound;
    }

    @Override
    public void visit(NuFormula formula) {
        RecursionVariable recursionVariable = formula.getRecursionVariable();
        Formula subFormula = formula.getFormula();

        if(!this.getFixedPointResults().containsKey(recursionVariable)) {
            this.getFixedPointResults().put(recursionVariable, this.getStates());
        }

        if (this.bound == Bound.MU_BOUNDED) {
            for (RecursionVariable boundedVariable: formula.getVariableMatcher().getBounded()) {
                this.getFixedPointResults().put(boundedVariable, this.getStates());
            }
        }

        Bound oldBound = this.bound;
        this.bound = Bound.NU_BOUNDED;
        this.putFormulaResult(formula, this.fixedPoint(subFormula, recursionVariable));
        this.bound = oldBound;
    }
}
