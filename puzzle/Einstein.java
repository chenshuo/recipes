import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Arrays;

import JaCoP.constraints.Alldifferent;
import JaCoP.constraints.Distance;
import JaCoP.constraints.XeqC;
import JaCoP.constraints.XeqY;
import JaCoP.constraints.XplusCeqZ;
import JaCoP.core.Domain;
import JaCoP.core.IntDomain;
import JaCoP.core.IntVar;
import JaCoP.core.Store;
import JaCoP.search.DepthFirstSearch;
import JaCoP.search.IndomainMin;
import JaCoP.search.InputOrderSelect;
import JaCoP.search.Search;
import JaCoP.search.SelectChoicePoint;

public class Einstein {
    static final int size = 5;
    static final Store store = new Store();
    static final IntVar kOne = new IntVar(store, 1, 1);

    public enum Nation {
        England, Sweden, Danmark, Norway, Germany;
        IntVar v = new IntVar(store, name(), 0, size - 1);
        static IntVar[] values = getValues(values());
    }

    public enum Drink {
        Tea, Coffee, Milk, Beer, Water;
        IntVar v = new IntVar(store, name(), 0, size - 1);
        static IntVar[] values = getValues(values());
    }

    public enum Color {
        Red, Green, Yellow, Blue, White;
        IntVar v = new IntVar(store, name(), 0, size - 1);
        static IntVar[] values = getValues(values());
    }

    public enum Pet {
        Dog, Bird, Horse, Cat, Fish;
        IntVar v = new IntVar(store, name(), 0, size - 1);
        static IntVar[] values = getValues(values());
    }

    public enum Cigar {
        PallMall, Dunhill, BlueMaster, Prince, Blends;
        IntVar v = new IntVar(store, name(), 0, size - 1);
        static IntVar[] values = getValues(values());
    }

    public static <T> IntVar[] getValues(T[] x) {
        IntVar[] values = new IntVar[x.length];
        for (int i = 0; i < values.length; ++i) {
            try {
                Field f = x[0].getClass().getDeclaredField("v");
                values[i] = (IntVar) f.get(x[i]);
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }
        return values;
    }

    public static void main(String[] args) throws Exception {
        store.impose(new Alldifferent(Nation.values));
        store.impose(new Alldifferent(Drink.values));
        store.impose(new Alldifferent(Color.values));
        store.impose(new Alldifferent(Pet.values));
        store.impose(new Alldifferent(Cigar.values));

        store.impose(new XeqY(Nation.England.v, Color.Red.v)); // 1
        store.impose(new XeqY(Nation.Sweden.v, Pet.Dog.v)); // 2
        store.impose(new XeqY(Nation.Danmark.v, Drink.Tea.v)); // 3
        store.impose(new XplusCeqZ(Color.Green.v, 1, Color.White.v)); // 4
        store.impose(new XeqY(Color.Green.v, Drink.Coffee.v)); // 5
        store.impose(new XeqY(Cigar.PallMall.v, Pet.Bird.v)); // 6
        store.impose(new XeqY(Color.Yellow.v, Cigar.Dunhill.v)); // 7
        store.impose(new XeqC(Drink.Milk.v, 2)); // 8
        store.impose(new XeqC(Nation.Norway.v, 0)); // 9
        store.impose(new Distance(Cigar.Blends.v, Pet.Cat.v, kOne)); // 10
        store.impose(new Distance(Cigar.Dunhill.v, Pet.Horse.v, kOne)); // 11
        store.impose(new XeqY(Cigar.BlueMaster.v, Drink.Beer.v)); // 12
        store.impose(new XeqY(Nation.Germany.v, Cigar.Prince.v)); // 13
        store.impose(new Distance(Nation.Norway.v, Color.Blue.v, kOne)); // 14
        store.impose(new Distance(Cigar.Blends.v, Drink.Water.v, kOne)); // 15

        run();
    }

    private static void run() {
        ArrayList<IntVar> all = new ArrayList<IntVar>();
        all.addAll(Arrays.asList(Nation.values));
        all.addAll(Arrays.asList(Drink.values));
        all.addAll(Arrays.asList(Color.values));
        all.addAll(Arrays.asList(Pet.values));
        all.addAll(Arrays.asList(Cigar.values));
        IntVar[] vars = new IntVar[all.size()];
        all.toArray(vars);

        Search<IntVar> search = new DepthFirstSearch<IntVar>();
        SelectChoicePoint<IntVar> select = new InputOrderSelect<IntVar>(
                store, vars, new IndomainMin<IntVar>());
        search.getSolutionListener().searchAll(true);
        search.getSolutionListener().recordSolutions(true);
        boolean result = search.labeling(store, select);
        if (result) {
            for (int i = 0; i < search.getSolutionListener().solutionsNo(); ++i) {
                Domain[] solution = search.getSolution(i + 1);
                print(vars, solution);
            }
            // search.getSolutionListener().printAllSolutions();
        }
    }

    private static void print(IntVar[] vars, Domain[] solution) {
        for (int i = 0; i < solution.length; i += size) {
            String[] names = new String[size];
            for (int j = 0; j < size; ++j) {
                IntDomain x = (IntDomain) solution[i + j];
                int idx = x.getElementAt(0);
                names[idx] = vars[i + j].id;
            }
            for (String name : names) {
                System.out.printf("%11s ", name);
            }
            System.out.println();
        }
        System.out.println();
    }
}
