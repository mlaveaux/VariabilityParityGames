package FTSMMC.SVPG;

import org.apache.commons.math3.distribution.BetaDistribution;

import java.util.Random;

public class BetaSetSizer implements SetSizer {
    BetaDistribution betadist;
    private Random r;

    public BetaSetSizer(float lambda) {
        betadist = new BetaDistribution(determineAlpha(lambda, 1), 1);
        this.r = new Random();
    }

    @Override
    public float getSetSize() {
        return (float) betadist.inverseCumulativeProbability(r.nextFloat());
    }

    private static float determineAlpha(float lambda, float beta) {
        return (lambda * beta) / (1f - lambda);
    }
}
