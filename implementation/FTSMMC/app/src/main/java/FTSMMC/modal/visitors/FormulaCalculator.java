package FTSMMC.modal.visitors;

import FTSMMC.modal.formulas.Formula;
import FTSMMC.models.State;

import java.util.Set;

public interface FormulaCalculator {
    Set<State> calculate(Formula formula);
}
