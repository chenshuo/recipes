package billing;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import billing.DataFields.UserField;

public class UserMonthUsage {
    private static final List<Package> emptyPackages = new ArrayList<Package>();
    public final int id;
    public final Map<DataFields.UserField, Object> data;

    public UserMonthUsage(int id, Map<DataFields.UserField, Object> data) {
        this.id = id;
        this.data = Collections.unmodifiableMap(data);
    }

    Object get(DataFields.UserField field) {
        return data.get(field);
    }

    boolean getBoolean(DataFields.UserField field) {
        Object obj = data.get(field);
        if (obj == null) {
            return false;
        } else {
            return (Boolean) obj;
        }
    }

    int getInteger(DataFields.UserField field) {
        Object obj = data.get(field);
        if (obj == null) {
            return 0;
        } else {
            return (Integer) obj;
        }
    }

    @SuppressWarnings("unchecked")
    List<Package> getPackages() {
        List<Package> packages = (List<Package>) data.get(UserField.kPackages);
        if (packages == null) {
            packages = emptyPackages;
        }
        return packages;
    }
}
