package mmc.aldebaran;

public class StrictParseException extends ParseException {
    public StrictParseException(String message, int index) {
        super(message, index);
    }
}
