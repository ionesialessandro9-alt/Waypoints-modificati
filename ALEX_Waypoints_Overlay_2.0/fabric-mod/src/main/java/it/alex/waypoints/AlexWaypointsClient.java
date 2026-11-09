
package it.alex.waypoints;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.reflect.TypeToken;

import net.fabricmc.api.ClientModInitializer;
import net.fabricmc.fabric.api.client.command.v2.ClientCommandManager;
import net.fabricmc.fabric.api.client.command.v2.ClientCommandRegistrationCallback;
import net.fabricmc.fabric.api.client.event.lifecycle.v1.ClientTickEvents;

import net.minecraft.client.MinecraftClient;
import net.minecraft.text.Text;

import java.lang.reflect.Type;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;

public class AlexWaypointsClient implements ClientModInitializer {

    private static final Gson GSON = new GsonBuilder().setPrettyPrinting().create();

    private static final Path CONFIG =
            MinecraftClient.getInstance().runDirectory.toPath().resolve("config");

    private static final Path WAYPOINT_FILE = CONFIG.resolve("alex-waypoints.json");
    private static final Path LIVE_FILE = CONFIG.resolve("alex-waypoints-live.json");

    private static final List<Waypoint> WAYPOINTS = new ArrayList<>();

    private static class Waypoint {
        String name;
        int x, y, z;

        Waypoint(String name, int x, int y, int z) {
            this.name = name;
            this.x = x;
            this.y = y;
            this.z = z;
        }
    }

    private static class LiveData {
        String name;
        int x, y, z;
        double distance;
        double playerX, playerY, playerZ;

        LiveData(String name, int x, int y, int z,
                 double distance, double px, double py, double pz) {
            this.name = name;
            this.x = x;
            this.y = y;
            this.z = z;
            this.distance = distance;
            this.playerX = px;
            this.playerY = py;
            this.playerZ = pz;
        }
    }

    @Override
    public void onInitializeClient() {
        load();

        ClientCommandRegistrationCallback.EVENT.register((dispatcher, registryAccess) -> {

            dispatcher.register(
                ClientCommandManager.literal("alexwp")

                    .then(ClientCommandManager.literal("add")
                        .then(ClientCommandManager.argument("name",
                                com.mojang.brigadier.arguments.StringArgumentType.word())
                            .then(ClientCommandManager.argument("x",
                                    com.mojang.brigadier.arguments.IntegerArgumentType.integer())
                                .then(ClientCommandManager.argument("y",
                                        com.mojang.brigadier.arguments.IntegerArgumentType.integer())
                                    .then(ClientCommandManager.argument("z",
                                            com.mojang.brigadier.arguments.IntegerArgumentType.integer())
                                        .executes(ctx -> {
                                            String n = com.mojang.brigadier.arguments.StringArgumentType.getString(ctx, "name");
                                            int x = com.mojang.brigadier.arguments.IntegerArgumentType.getInteger(ctx, "x");
                                            int y = com.mojang.brigadier.arguments.IntegerArgumentType.getInteger(ctx, "y");
                                            int z = com.mojang.brigadier.arguments.IntegerArgumentType.getInteger(ctx, "z");

                                            WAYPOINTS.removeIf(w -> w.name.equalsIgnoreCase(n));
                                            WAYPOINTS.add(new Waypoint(n, x, y, z));

                                            save();
                                            tell("Waypoint salvato: " + n);
                                            return 1;
                                        })
                                    )
                                )
                            )
                        )
                    )

                    .then(ClientCommandManager.literal("list")
                        .executes(ctx -> {
                            for (Waypoint w : WAYPOINTS) {
                                tell(w.name + " X:" + w.x + " Y:" + w.y + " Z:" + w.z);
                            }

                            if (WAYPOINTS.isEmpty()) {
                                tell("Nessun waypoint salvato.");
                            }

                            return 1;
                        })
                    )

                    .then(ClientCommandManager.literal("remove")
                        .then(ClientCommandManager.argument("name",
                                com.mojang.brigadier.arguments.StringArgumentType.word())
                            .executes(ctx -> {
                                String n = com.mojang.brigadier.arguments.StringArgumentType.getString(ctx, "name");

                                boolean ok = WAYPOINTS.removeIf(w -> w.name.equalsIgnoreCase(n));
                                save();

                                tell(ok ? "Rimosso: " + n : "Waypoint non trovato.");
                                return 1;
                            })
                        )
                    )
            );
        });

        ClientTickEvents.END_CLIENT_TICK.register(client -> writeLiveData());
    }

    private static void writeLiveData() {
        MinecraftClient mc = MinecraftClient.getInstance();

        if (mc.player == null || mc.world == null) {
            return;
        }

        try {
            Files.createDirectories(CONFIG);

            LiveData data = null;
            double best = Double.MAX_VALUE;

            for (Waypoint w : WAYPOINTS) {
                double dx = mc.player.getX() - (w.x + 0.5);
                double dy = mc.player.getY() - w.y;
                double dz = mc.player.getZ() - (w.z + 0.5);

                double d = Math.sqrt(dx * dx + dy * dy + dz * dz);

                if (d < best) {
                    best = d;
                    data = new LiveData(
                            w.name, w.x, w.y, w.z, d,
                            mc.player.getX(), mc.player.getY(), mc.player.getZ()
                    );
                }
            }

            if (data == null) {
                data = new LiveData(
                        "Nessun waypoint", 0, 0, 0, 0,
                        mc.player.getX(), mc.player.getY(), mc.player.getZ()
                );
            }

            Files.writeString(LIVE_FILE, GSON.toJson(data));

        } catch (Exception e) {
            System.err.println("[ALEX] Live data error: " + e.getMessage());
        }
    }

    private static void load() {
        try {
            Files.createDirectories(CONFIG);

            if (Files.exists(WAYPOINT_FILE)) {
                Type type = new TypeToken<List<Waypoint>>() {}.getType();

                List<Waypoint> list =
                        GSON.fromJson(Files.readString(WAYPOINT_FILE), type);

                if (list != null) {
                    WAYPOINTS.addAll(list);
                }
            }

        } catch (Exception e) {
            System.err.println("[ALEX] Load error: " + e.getMessage());
        }
    }

    private static void save() {
        try {
            Files.createDirectories(CONFIG);
            Files.writeString(WAYPOINT_FILE, GSON.toJson(WAYPOINTS));

        } catch (Exception e) {
            tell("Errore salvataggio waypoint.");
        }
    }

    private static void tell(String message) {
        MinecraftClient mc = MinecraftClient.getInstance();

        if (mc.player != null) {
            mc.player.sendMessage(Text.literal("[ALEX] " + message), false);
        }
    }
}
