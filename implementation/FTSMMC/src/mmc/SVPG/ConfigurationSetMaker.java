package mmc.SVPG;

import mmc.features.FeatureDiagram;
import net.sf.javabdd.BDD;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class ConfigurationSetMaker implements SetMaker {
    public List<BDD> collectionItems;

    public ConfigurationSetMaker(){
        int nrofconfs = (1 << FeatureDiagram.PrimaryFD.varCount());
        collectionItems = new ArrayList<>();
        for(int i = 0;i<nrofconfs;i++) {
            BDD confbdd = FeatureDiagram.PrimaryFD.factory.one();
            int conf = i;
            for (int j = FeatureDiagram.PrimaryFD.varCount() - 1; j >= 0; j--) {
                if ((1 << j) <= conf) {
                    conf -= (1 << j);
                    BDD a = FeatureDiagram.PrimaryFD.factory.ithVar(j);
                    confbdd.andWith(a.id());
                } else {
                    BDD a = FeatureDiagram.PrimaryFD.factory.ithVar(j);
                    confbdd.andWith(a.id().not().id());
                }
            }
            collectionItems.add(confbdd);
        }
    }
    @Override
    public BDD makeSet(float r) {
        BDD result;
        int nrofconfs = (1 << FeatureDiagram.PrimaryFD.varCount());
        Collections.shuffle(collectionItems);
        if(r < 0.5) {
            result = FeatureDiagram.PrimaryFD.factory.zero();
            nrofconfs = (int) ((float) nrofconfs * r);
            nrofconfs = Math.max(nrofconfs, 1);
            for (int i = 0; i < nrofconfs; i++) {
                result.orWith(collectionItems.get(i).id());
            }
        } else {
            result = FeatureDiagram.PrimaryFD.factory.one();
            int remove = (int) ((float) nrofconfs * r);
            remove = Math.max(remove, 1);
            remove = nrofconfs - remove;
            for (int i = 0; i < remove; i++) {
                result.andWith(collectionItems.get(i).id().not().id());
            }
        }
        return result;
    }
}
