package mmc.modal.visitors;

import mmc.modal.formulas.*;
import mmc.models.Label;
import mmc.models.Lts;
import mmc.models.State;

import java.util.*;
import java.util.stream.Collectors;

public class NaiveAlgorithm implements FormulaCalculator, FormulaVisitor {
    private final Lts lts;
    private final Set<State> states;
    private final Map<Formula, Set<State>> results;
    private final Map<RecursionVariable, Set<State>> fixedPointResults;

    public NaiveAlgorithm(Lts lts) {
        Objects.requireNonNull(lts);
        this.lts = lts;
        this.states = new HashSet<>(Arrays.asList(lts.getStates()));
        this.results = new HashMap<>();
        this.fixedPointResults = new HashMap<>();
    }

    protected Lts getLts() {
        return this.lts;
    }

    protected Set<State> getStates() {
        return this.states;
    }

    protected Map<RecursionVariable, Set<State>> getFixedPointResults() {
        return this.fixedPointResults;
    }

    @Override
    public void visit(BoxFormula formula) {
        Set<State> formulaResult = this.getFormulaResult(formula.getFormula());
        Label action = formula.getAction();
        Set<State> result = Arrays.stream(this.lts.getStates())
            .filter(startState -> !startState.getTransitionLabels().contains(action) ||
                startState.transition(action).stream()
                    .allMatch(formulaResult::contains))
            .collect(Collectors.toSet());
        this.putFormulaResult(formula, result);
    }

    @Override
    public void visit(DiamondFormula formula) {
        Set<State> formulaResult = this.getFormulaResult(formula.getFormula());
        Label action = formula.getAction();
        Set<State> result = Arrays.stream(this.lts.getStates())
            .filter(startState -> startState.getTransitionLabels().contains(action) &&
                startState.transition(action).stream()
                    .anyMatch(formulaResult::contains))
            .collect(Collectors.toSet());
        this.putFormulaResult(formula, result);
    }

    @Override
    public void visit(LiteralFalse formula) {
        this.putFormulaResult(formula, new HashSet<>());
    }

    @Override
    public void visit(LiteralTrue formula) {
        this.putFormulaResult(formula, states);
    }

    @Override
    public void visit(LogicAndFormula formula) {
        Set<State> leftResult = this.getFormulaResult(formula.getLeft());
        Set<State> rightResult = this.getFormulaResult(formula.getRight());
        HashSet<State> result = new HashSet<>(leftResult);
        result.retainAll(rightResult);
        this.putFormulaResult(formula, result);
    }

    @Override
    public void visit(LogicOrFormula formula) {
        Set<State> leftResult = this.getFormulaResult(formula.getLeft());
        Set<State> rightResult = this.getFormulaResult(formula.getRight());
        HashSet<State> result = new HashSet<>(leftResult);
        result.addAll(rightResult);
        this.putFormulaResult(formula, result);
    }

    @Override
    public void visit(MuFormula formula) {
        RecursionVariable recursionVariable = formula.getRecursionVariable();
        Formula subFormula = formula.getFormula();
        this.fixedPointResults.put(recursionVariable, new HashSet<>());
        this.putFormulaResult(formula, this.fixedPoint(subFormula, recursionVariable));
    }

    @Override
    public void visit(NuFormula formula) {
        RecursionVariable recursionVariable = formula.getRecursionVariable();
        Formula subFormula = formula.getFormula();
        this.fixedPointResults.put(recursionVariable, new HashSet<>(this.states));
        this.putFormulaResult(formula, this.fixedPoint(subFormula, recursionVariable));
    }

    public Set<State> fixedPoint(Formula subFormula, RecursionVariable recursionVariable) {
        int i = 0;
        Boolean equilibrium;
        do {
            subFormula.accept(this);
            Set<State> result = this.results.get(subFormula);
            Set<State> previousResult = this.fixedPointResults.put(recursionVariable, result);
            equilibrium = result.equals(previousResult);
            i++;
        } while(!equilibrium);
        System.out.println(String.format("Calculated %s in %d iterations.", recursionVariable, i));
        return this.fixedPointResults.get(recursionVariable);
    }

    @Override
    public void visit(RecursionVariable formula) {
        this.putFormulaResult(formula, fixedPointResults.get(formula));
    }

    @Override
    public Set<State> calculate(Formula formula) {
        return this.getFormulaResult(formula);
    }

    public void clear() {
        this.results.clear();
    }

    protected Set<State> getFormulaResult(Formula formula) {
        formula.accept(this);
        return this.results.get(formula);
    }

    protected void putFormulaResult(Formula formula, Set<State> result) {
        this.results.put(formula, result);
    }
}
