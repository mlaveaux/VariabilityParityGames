package mmc.modal.visitors;

import mmc.modal.formulas.Formula;
import mmc.models.State;

import java.util.Set;

public interface FormulaCalculator {
    Set<State> calculate(Formula formula);
}
