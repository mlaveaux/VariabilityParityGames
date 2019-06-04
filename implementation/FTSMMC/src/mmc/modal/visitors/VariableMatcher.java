package mmc.modal.visitors;

import mmc.modal.formulas.*;

import java.util.HashSet;
import java.util.Objects;
import java.util.Set;

public class VariableMatcher extends RecursiveVisitor implements FormulaVisitor {
    private final RecursionVariable variable;
    private final Set<RecursionVariable> free;
    private final Set<RecursionVariable> bounded;
    private final Set<RecursionVariable> all;

    private VariableMatcher(RecursionVariable variable) {
        Objects.requireNonNull(variable);
        this.variable = variable;
        this.free = new HashSet<>();
        this.bounded = new HashSet<>();
        this.all = new HashSet<>();
    }

    public static VariableMatcher findVariables(FixedPointFormula formula) {
        VariableMatcher matcher = new VariableMatcher(formula.getRecursionVariable());
        formula.getFormula().accept(matcher);

        return matcher;
    }

    public Set<RecursionVariable> getFree() {
        return this.free;
    }

    public Set<RecursionVariable> getBounded() {
        return this.bounded;
    }

    public Set<RecursionVariable> getAll() {
        return  this.all;
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
        formula.findVariables();
        VariableMatcher variableMatcher = formula.getVariableMatcher();
        variableMatcher.getFree().stream()
                .filter(other -> !this.variable.equals(other))
                .map(this.free::add);
        this.bounded.addAll(variableMatcher.bounded);
        this.bounded.remove(formula.getRecursionVariable());
        this.all.addAll(variableMatcher.getAll());
    }

    @Override
    public void visit(NuFormula formula) {
        formula.findVariables();
        VariableMatcher variableMatcher = formula.getVariableMatcher();
        variableMatcher.getFree().stream()
                .filter(other -> !this.variable.equals(other))
                .map(this.free::add);
        this.bounded.addAll(variableMatcher.bounded);
        this.bounded.remove(formula.getRecursionVariable());
        this.all.addAll(variableMatcher.getAll());
    }

    @Override
    public void visit(RecursionVariable formula) {
        this.all.add(formula);
        if(this.variable.equals(formula)) {
            this.bounded.add(formula);
        } else {
            this.free.add(formula);
        }
    }
}
