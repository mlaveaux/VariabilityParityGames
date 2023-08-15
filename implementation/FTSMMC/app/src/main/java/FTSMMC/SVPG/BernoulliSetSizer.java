package FTSMMC.SVPG;

import java.util.Random;

public class BernoulliSetSizer implements SetSizer {
    private float p;
    private Random r;

    public BernoulliSetSizer(float lambda) {
        this.p = determineP(lambda);
        this.r = new Random();
    }

    @Override
    public float getSetSize() {
        if (r.nextFloat() < p)
            return 1;
        else
            return 0.5f;
    }

    private static float determineP(float lambda) {
        return 2 * lambda - 1;
    }
}
