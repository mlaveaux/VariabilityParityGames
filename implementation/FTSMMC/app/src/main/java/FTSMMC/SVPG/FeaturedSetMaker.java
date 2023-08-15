package FTSMMC.SVPG;

import FTSMMC.features.FeatureDiagram;
import net.sf.javabdd.BDD;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Random;

public class FeaturedSetMaker implements SetMaker {
    private Random random;
    private List<Integer> l;

    public FeaturedSetMaker() {
        random = new Random();
        l = new ArrayList<>();
        for (int i = 0; i < FeatureDiagram.PrimaryFD.varCount(); i++) {
            l.add(i);
        }
    }

    @Override
    public BDD makeSet(float r) {
        int cutoff = (int) Math.min(FeatureDiagram.PrimaryFD.varCount(), Math.round(-1 * Math.log(r) / Math.log(2)));
        BDD result = FeatureDiagram.PrimaryFD.FD.id();
        Collections.shuffle(l);
        for (int i = 0; i < cutoff; i++) {
            if (random.nextBoolean())
                result.andWith(FeatureDiagram.PrimaryFD.factory.ithVar(l.get(i)).id());
            else
                result.andWith(FeatureDiagram.PrimaryFD.factory.ithVar(l.get(i)).id().not().id());
        }
        return result;
    }
}
